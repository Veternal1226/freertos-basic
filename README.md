
This branch is used to implement porting freertos-basic onto STM32F4 Discovery.

We adpated TheKK/myFreeRTOS. The source code of FreeRTOS kernel are in folder FreeRTOS. Application codes are in CORTEX_M4F_STM32F407ZG-SK/app and main.c in the same folder.

Version of kernel is 8.2.1.


In Utilities/, **STM32F429I-Discovery** include library in **Common/** and **Third_Party**. As a result, do not remove them even if they were not mentioned in Makefile.