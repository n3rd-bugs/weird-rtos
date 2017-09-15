/*
 * kernel_target.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
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

/* Target toolset configuration. */
#define TOOL_AVR_GCC        0x01
#define TOOL_ARM_GCC        0x02

/* RTOS configuration. */
#ifndef CMAKE_BUILD
#define PLAT_TARGET         PLAT_ATMEGAXX4
#define RTOS_TARGET         TARGET_AVR
#define TOOL_TARGET         TOOL_AVR_GCC
#endif

/* Toolset includes. */
#if (TOOL_TARGET == TOOL_AVR_GCC)
#include <avr_gcc.h>
#elif (TOOL_TARGET == TOOL_ARM_GCC)
#include <arm_gcc.h>
#endif

/* Processor includes. */
#if (RTOS_TARGET == TARGET_AVR)
#include <avr.h>
#elif (RTOS_TARGET == TARGET_CORTEX_M3)
#include <cortex_m3.h>
#elif (RTOS_TARGET == TARGET_CORTEX_M4)
#include <cortex_m4.h>
#endif

/* Platform includes. */
#if (PLAT_TARGET == PLAT_ATMEGAXX4)
#include <atmegaxx4.h>
#elif (PLAT_TARGET == PLAT_STM32F407_DISC)
#include <stm32f407.h>
#endif

#include <p_string.h>
#include <tasks.h>

/* Following functions must be implemented by target porting layer. */
void system_tick_Init(void);
void stack_init(TASK *, TASK_ENTRY *, void *);

/* This must be implemented by application. */
int main(void);

#endif /* _KERNEL_TARGET_H_ */
