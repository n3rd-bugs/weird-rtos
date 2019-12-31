/*
 * stm32f411.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _STM32F411_H_
#define _STM32F411_H_

#include <kernel.h>
#include <stm32f411xe.h>

/* Peripheral clock configuration. */
#define SYS_FREQ                        84000000
#define PCLK1_FREQ                      (SYS_FREQ / 2)
#define PCLK2_FREQ                      (SYS_FREQ)
#define HW_TICKS_PER_SEC                (PCLK1_FREQ)

/* Required definitions for scheduling. */
#define CORTEX_M4_PEND_SV_REG           (SCB->ICSR)
#define CORTEX_M4_PEND_SV_MASK          (SCB_ICSR_PENDSVSET_Msk)
#define CORTEX_M4_INT_SYS_PRI           (0xFF)
#define CORTEX_M4_SET_PENDSV_PRI()      NVIC_SetPriority(PendSV_IRQn, CORTEX_M4_INT_SYS_PRI)

/* Control FPU support. */
#define CORTEX_M4_FPU                   (TRUE)

/* End of BSS marks the start of system stack. */
extern uint32_t _ebss;

/* Memory configuration. */
#define SYSTEM_STACK                    (((uint8_t *)&_ebss) + TARGET_HEAP_SIZE)
#define STM32F411_STACK_END             (0x20020000)
#define SYS_STACK_SIZE                  (STM32F411_STACK_END - (uint32_t)SYSTEM_STACK)

/* System registers. */
#define STM32_UUID                      ((uint8_t *)0x1FFF7A10)

#ifdef CONFIG_SLEEP
/* Function prototypes. */
uint64_t current_hardware_tick(void);
#endif /* CONFIG_SLEEP */

/* Helper functions. */
void system_entry(void);
void sysclock_init(void);
void wdt_disbale(void);

#endif /* _STM32F411_H_ */
