PROJECT = myRTOS

EXECUTABLE = $(PROJECT).elf
BIN_IMAGE = $(PROJECT).bin
#HEX_IMAGE = $(PROJECT).hex

# set the path to STM32F429I-Discovery firmware package
# STDP ?= ../STM32F429I-Discovery_FW_V1.0.1

# Toolchain configurations
CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size
GDB = $(CROSS_COMPILE)gdb

# CFLAGS Reference: https://gcc.gnu.org/onlinedocs/gcc/ARM-Options.html

# Cortex-M4 implements the ARMv7E-M architecture
CPU = cortex-m4
CFLAGS = -mcpu=$(CPU) -march=armv7e-m -mtune=cortex-m4
CFLAGS += -mlittle-endian -mthumb
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -O0

# Basic configurations
CFLAGS += -g -std=c99
CFLAGS += -Wall

# Optimizations
# CFLAGS += -ffast-math
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-common
CFLAGS += --param max-inline-insns-single=1000

# specify STM32F429
CFLAGS += -DSTM32F429_439xx

# STM32F4xx_StdPeriph_Driver
#CFLAGS += -DUSE_STDPERIPH_DRIVER

RTOS = $(PWD)/freertos/FreeRTOS
CFLAGS += -I $(PWD)/source \
		-I $(PWD)/source/lib \
		-I $(PWD)/source/startup \
		-I $(RTOS)/include \
		-I $(RTOS)/portable/GCC/ARM_CM4F \
		-I $(PWD)/freertos/CMSIS/Include \
#		-I $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/inc \
		-I $(PWD)/Utilities/STM32F429I-Discovery 
#SEMIHOSTING_FLAGS = --specs=rdimon.specs -lc -lrdimon

define get_library_path
    $(shell dirname $(shell $(CC) $(CFLAGS) -print-file-name=$(1)))
endef
LDFLAGS += -L $(call get_library_path,libc.a)
LDFLAGS += -L $(call get_library_path,libgcc.a)
LDFLAGS += -T $(PWD)/source/stm32f429zi_flash.ld
LDFLAGS += --gc-sections

# STARTUP FILE
OBJS += $(PWD)/source/startup_stm32f429_439xx.o

#My restart
OBJS += \
      $(PWD)/source/main.o \
      $(PWD)/source/startup/system_stm32f4xx.o

OBJS += \
      $(RTOS)/croutine.o \
      $(RTOS)/event_groups.o \
      $(RTOS)/list.o \
      $(RTOS)/queue.o \
      $(RTOS)/tasks.o \
      $(RTOS)/timers.o \
      $(RTOS)/portable/GCC/ARM_CM4F/port.o \
      $(RTOS)/portable/MemMang/heap_1.o \

#OBJS += \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/misc.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.o \
    $(PWD)/freertos/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.o \
    $(PWD)/Utilities/STM32F429I-Discovery/stm32f429i_discovery.o \
    $(PWD)/Utilities/STM32F429I-Discovery/stm32f429i_discovery_sdram.o \
    $(PWD)/Utilities/STM32F429I-Discovery/stm32f429i_discovery_lcd.o \
    $(PWD)/Utilities/STM32F429I-Discovery/stm32f429i_discovery_ioe.o

#Custom C Library
#OBJS += \
	$(PWD)/source/lib/clib.o \
	$(PWD)/source/lib/dir.o \
	$(PWD)/source/lib/filesystem.o \
	$(PWD)/source/lib/fio.o \
	$(PWD)/source/lib/mouse.o \
	$(PWD)/source/lib/osdebug.o \
	$(PWD)/source/lib/romfs.o \
	$(PWD)/source/lib/shell.o \
	$(PWD)/source/lib/hash-djb2.o \
	$(PWD)/source/lib/host.o \
	$(PWD)/source/lib/mmtest.o \
	$(PWD)/source/lib/string-util.o \
	#$(PWD)/source/lib/stm32_p103.o \
    	#$(PWD)/source/lib/main.o

all: $(BIN_IMAGE)

$(BIN_IMAGE): $(EXECUTABLE)
	$(OBJCOPY) -O binary $^ $@
#	$(OBJCOPY) -O ihex $^ $(HEX_IMAGE)
	$(OBJDUMP) -h -S -D $(EXECUTABLE) > $(PROJECT).lst
	$(SIZE) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJS)
	$(LD) -o $@ $(OBJS) \
		--start-group $(LIBS) --end-group \
		$(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
#	$(CC) $(SEMIHOSTING_FLAGS) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@
#	$(CC) $(SEMIHOSTING_FLAGS) $(CFLAGS) -c $< -o $@

flash:
	st-flash write $(BIN_IMAGE) 0x8000000

.PHONY: clean
clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(BIN_IMAGE)
	rm -rf $(HEX_IMAGE)
	rm -f $(OBJS)
	rm -f $(PROJECT).lst

gdb: all
	$(GDB) $(EXECUTABLE)

openocd:
	openocd -f board/stm32f4discovery.cfg
 