/*
 * kernel_target.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _KERNEL_TARGET_H_
#define _KERNEL_TARGET_H_

/* Target processor definitions. */
#define TARGET_AVR          0x1
#define TARGET_CORTEX_M3    0x2
#define TARGET_CORTEX_M4    0x3

/* Target platform definitions. */
#define PLAT_ATMEGAXX4      0x01
#define PLAT_STM32F407_DISC 0x02
#define PLAT_STM32F103C8T6  0x03

/* Target toolset configuration. */
#define TOOL_AVR_GCC        0x01
#define TOOL_ARM_GCC        0x02

/* RTOS configuration. */
#ifndef CMAKE_BUILD
#define TARGET_PLATFORM     PLAT_ATMEGAXX4
#define TARGET_CPU          TARGET_AVR
#define TARGET_TOOLS        TOOL_AVR_GCC
#endif

/* Toolset includes. */
#if (TARGET_TOOLS == TOOL_AVR_GCC)
#include <avr_gcc.h>
#elif (TARGET_TOOLS == TOOL_ARM_GCC)
#include <arm_gcc.h>
#endif

/* Processor includes. */
#if (TARGET_CPU == TARGET_AVR)
#include <avr.h>
#elif (TARGET_CPU == TARGET_CORTEX_M3)
#include <cortex_m3.h>
#elif (TARGET_CPU == TARGET_CORTEX_M4)
#include <cortex_m4.h>
#endif

/* Platform includes. */
#if (TARGET_PLATFORM == PLAT_ATMEGAXX4)
#include <atmegaxx4.h>
#elif (TARGET_PLATFORM == PLAT_STM32F407_DISC)
#include <stm32f407.h>
#elif (TARGET_PLATFORM == PLAT_STM32F103C8T6)
#include <stm32f103.h>
#endif

#include <p_string.h>
#include <tasks.h>

/* Following functions must be implemented by target porting layer. */
void system_tick_Init(void);
void stack_init(TASK *, TASK_ENTRY *, void *);

/* This must be implemented by application. */
int main(void);

#endif /* _KERNEL_TARGET_H_ */
