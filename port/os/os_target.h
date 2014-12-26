/*
 * os_target.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef OS_TARGET_H
#define OS_TARGET_H

#define TARGET_AVR          0x1
#define TARGET_CORTEX_M3    0x2

#define RTOS_TARGET         TARGET_AVR

/* Target includes. */
#if (RTOS_TARGET == TARGET_AVR)
#include <os_avr.h>
#endif
#if (RTOS_TARGET == TARGET_CORTEX_M3)
#include <os_cortex_m3.h>
#include <MK40DZ10.h>
#endif

#include <tasks.h>

/* Following functions must be implemented by target porting layer. */
void system_tick_Init();
void os_stack_init(TASK *, TASK_ENTRY *, void *);

/* This must be implemented by application. */
int main(void);

#endif /* OS_TARGET_H */
