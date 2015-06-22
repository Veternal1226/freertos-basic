#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"
#include "stm32_p103.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

/* Filesystem includes */
#include "filesystem.h"
#include "fio.h"
#include "romfs.h"

#include "clib.h"
#include "shell.h"
#include "host.h"


unsigned int get_time();
unsigned int get_reload();
unsigned int get_current();


/* _sromfs symbol can be found in main.ld linker script
 * it contains file system structure of test_romfs directory
 */
extern const unsigned char _sromfs;

//static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
/* Add for serial input */
volatile xQueueHandle serial_rx_queue = NULL;

/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */
void USART2_IRQHandler()
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;

	/* If this interrupt is for a transmit... */
	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
		/* "give" the serial_tx_wait_sem semaphore to notfiy processes
		 * that the buffer has a spot free for the next byte.
		 */
		xSemaphoreGiveFromISR(serial_tx_wait_sem, &xHigherPriorityTaskWoken);

		/* Diables the transmit interrupt. */
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		/* If this interrupt is for a receive... */
	}else if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		char msg = USART_ReceiveData(USART2);

		/* If there is an error when queueing the received byte, freeze! */
		if(!xQueueSendToBackFromISR(serial_rx_queue, &msg, &xHigherPriorityTaskWoken))
			while(1);
	}
	else {
		/* Only transmit and receive interrupts should be enabled.
		 * If this is another type of interrupt, freeze.
		 */
		while(1);
	}

	if (xHigherPriorityTaskWoken) {
		taskYIELD();
	}
}

void send_byte(char ch)
{
	/* Wait until the RS232 port can receive another byte (this semaphore
	 * is "given" by the RS232 port interrupt when the buffer has room for
	 * another byte.
	 */
	while (!xSemaphoreTake(serial_tx_wait_sem, portMAX_DELAY));

	/* Send the byte and enable the transmit interrupt (it is disabled by
	 * the interrupt).
	 */
	USART_SendData(USART2, ch);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

char recv_byte()
{
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	char msg;
	while(!xQueueReceive(serial_rx_queue, &msg, portMAX_DELAY));
	return msg;
}
void command_prompt(void *pvParameters)
{
	char buf[128];
	char *argv[20];
    char hint[] = USER_NAME "@" USER_NAME "-STM32:~$ ";

	fio_printf(1, "\rWelcome to FreeRTOS Shell\r\n");
	while(1){
                fio_printf(1, "%s", hint);
		fio_read(0, buf, 127);
	
		int n=parse_command(buf, argv);

		/* will return pointer to the command function */
		cmdfunc *fptr=do_command(argv[0]);
		if(fptr!=NULL)
			fptr(n, argv);
		else
			fio_printf(2, "\r\n\"%s\" command not found.\r\n", argv[0]);
	}

}
/* 
 * Calculate the context switch time by vTaskDelay
 */
void cal_ct(void *pvParameters)
{
	
        fio_printf(1,"now time from scheduler started to now = %d\r\n",get_time());   
        int i = 0;
	int record[10]={0};
        for(; i < 10 ;i++){
        /* 
         * Use tick is more precisely than ms
         */
		vTaskDelay(1000 / portTICK_RATE_MS);
        	//vTaskDelay(100);
        	//fio_printf(1,"now time from scheduler started to now = %d\r\n",get_time());   
		record[i] = get_time();
        }
	
        for(i = 0; i < 10 ;i++){
        	fio_printf(1,"now time from scheduler started to now = %d\r\n",record[i]);   
	}
}
/* 
 * Calculate the context switch time by vTaskDelayUntil
 */
void cal_ct2( void * pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 10;

	// Initialise the xLastWakeTime variable with the current time.
        xLastWakeTime = xTaskGetTickCount();

        for( ;; )
        {
            // Wait for the next cycle.
            vTaskDelayUntil( &xLastWakeTime, xFrequency );

            // Perform action here.
            fio_printf(1,"now time from scheduler started to now = %d\r\n",get_time());   
        }
}
void system_logger(void *pvParameters)
{
    char buf[128];
    char output[512] = {0};
    char *tag = "\nName          State   Priority  Stack  Num\n*******************************************\n";
    int handle, error;
    const portTickType xDelay = 100000 / 100;

    handle = host_action(SYS_OPEN, "output/syslog", 4);
    if(handle == -1) {
        fio_printf(1, "Open file error!\n");
        return;
    }

    while(1) {
        memcpy(output, tag, strlen(tag));
        error = host_action(SYS_WRITE, handle, (void *)output, strlen(output));
        if(error != 0) {
            fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
            host_action(SYS_CLOSE, handle);
            return;
        }
        vTaskList(buf);

        memcpy(output, (char *)(buf + 2), strlen((char *)buf) - 2);

        error = host_action(SYS_WRITE, handle, (void *)buf, strlen((char *)buf));
        if(error != 0) {
            fio_printf(1, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
            host_action(SYS_CLOSE, handle);
            return;
        }

        vTaskDelay(xDelay);
    }
    
    host_action(SYS_CLOSE, handle);
}

int main()
{
	init_rs232();
	enable_rs232_interrupts();
	enable_rs232();
	
	fs_init();
	fio_init();
	
	register_romfs("romfs", &_sromfs);
	
	/* Create the queue used by the serial task.  Messages for write to
	 * the RS232. */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	/* Add for serial input 
	 * Reference: www.freertos.org/a00116.html */
	serial_rx_queue = xQueueCreate(1, sizeof(char));

    register_devfs();
	/* Create a task to output text read from romfs. */
	xTaskCreate(command_prompt,
	            (portCHAR *) "CLI",
	            512 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL);

#if 0
	/* Create a task to record system log. */
	xTaskCreate(system_logger,
	            (signed portCHAR *) "Logger",
	            1024 /* stack size */, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif


        xTaskCreate(cal_ct,(portCHAR *) "ct",1024,NULL,tskIDLE_PRIORITY + 3,NULL);
	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

void vApplicationTickHook()
{
}


/* 
 * time related function 
 */
unsigned int get_reload()
{
        return *(uint32_t *) 0xE000E014;
}

unsigned int get_current()
{
        return *(uint32_t *) 0xE000E018;
}

unsigned int get_time()
{
        static unsigned int const *reload = (void *) 0xE000E014;
        static unsigned int const *current = (void *) 0xE000E018;
        static const unsigned int scale = 1000000 / configTICK_RATE_HZ;
                                        /* microsecond */
        return xTaskGetTickCount() * scale + (*reload - *current) * (1.0) / (*reload / scale);
}

