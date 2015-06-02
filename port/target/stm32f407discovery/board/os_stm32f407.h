/*
 * os_stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#ifndef OS_STM32F407_H
#define OS_STM32F407_H

#include <os.h>
#include <stm32f407xx.h>
#include <usart_stm32f407.h>

/* Peripheral clock configuration. */
#define SYS_FREQ                168000000
#define PCLK_FREQ               84000000

/* Required definitions for scheduling. */
#define CORTEX_M4_PEND_SV_REG           (SCB->ICSR)
#define CORTEX_M4_PEND_SV_MAST          (SCB_ICSR_PENDSVSET_Msk)
#define CORTEX_M4_INT_PEND_SV_PRI_REG   (SCB->SHP[10])
#define CORTEX_M4_SYS_TICK_REG          (SysTick->CTRL)
#define CORTEX_M4_SYS_TICK_MASK         (SysTick_CTRL_TICKINT_Msk)
#define CORTEX_M4_INT_SYS_TICK_PRI_REG  (SCB->SHP[11])
#define CORTEX_M4_INT_SYS_PRI           (0x1)

#define CORTEX_M4_FPU                   (TRUE)

/* 64-bit clock management. */
#define current_hardware_tick()         pit_get_clock()
#define current_hardware_tick_usec()    (pit_get_clock() / PCLK_FREQ)

/* Hook-up the STDIO printf function. */
#ifdef printf
#undef printf
#endif
#define printf stm32f407_printf

/* Function prototypes. */
uint64_t pit_get_clock();
int32_t stm32f407_printf(char *, ...);

/* Helper functions. */
void system_entry(void);
void sysclock_init();
void wdt_disbale();

#endif /* OS_STM32F407_H */
