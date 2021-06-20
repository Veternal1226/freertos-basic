//#include "stm32f4xx.h"
//#include "stm32_p103.h"
/* Scheduler includes. */
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

/* Filesystem includes */
/*#include "filesystem.h"
#include "fio.h"
#include "romfs.h"

#include "clib.h"
#include "shell.h"
#include "host.h"*/

/* _sromfs symbol can be found in main.ld linker script
 * it contains file system structure of test_romfs directory
 */
//extern const unsigned char _sromfs;

//static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
/* Add for serial input */
volatile xQueueHandle serial_rx_queue = NULL;

void nothing(){
	int i;
	for(i=0;i<10000;i++){}

}

/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */

int main()
{

#if 0
	//init_rs232();
	//enable_rs232_interrupts();
	//enable_rs232();
	fs_init();
	fio_init();
	//register_romfs("romfs", &_sromfs);
#endif	

//prvInit();

#if 0 //test LCD Library
LCD_DrawFullCircle(100, 100, 50); // (x, y, radius)

//LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr);
uint8_t str[] = {'H', 'i', '!', ' ', 'V', 'e', 'c', 'k'};
LCD_DisplayStringLine(0, str);

//LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c);
// uint16_t Xpos = 23;
// uint16_t Ypos = 220;
// const uint16_t c = 'A';
// LCD_DrawChar(Xpos, Ypos, &c);

//LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii);
//LCD_DisplayChar(170, 45, 0x97);
#endif	
	/* Create the queue used by the serial task.  Messages for write to
	 * the RS232. */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	/* Add for serial input 
	 * Reference: www.freertos.org/a00116.html */
	serial_rx_queue = xQueueCreate(1, sizeof(char));

	//register_devfs();
	/* Create a task to output text read from romfs. */
	xTaskCreate(nothing,
	            (portCHAR *) "CLI",
	            512 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL);

#if 0
	/* Create a task to record system log. */
	xTaskCreate(system_logger,
	            (signed portCHAR *) "Logger",
	            1024 /* stack size */, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif

	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

void vApplicationTickHook()
{
}
