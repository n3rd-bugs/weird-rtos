/*
 * stm32f103.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

#ifndef _STM32F103_H_
#define _STM32F103_H_

#include <kernel.h>
#define STM32F10X_MD
#include <stm32f10x.h>

/* Peripheral clock configuration. */
#define SYS_FREQ                        (72000000)
#define PCLK1_FREQ                      (SYS_FREQ / 2)
#define PCLK2_FREQ                      (SYS_FREQ)
#define HW_TICKS_PER_SEC                (PCLK1_FREQ)

/* Required definitions for scheduling. */
#define CORTEX_M3_PEND_SV_REG           (SCB->ICSR)
#define CORTEX_M3_PEND_SV_MASK          (SCB_ICSR_PENDSVSET_Msk)
#define CORTEX_M3_INT_PEND_SV_PRI       (0xFF)
#define CORTEX_M3_SET_PENDSV_PRI()      NVIC_SetPriority(PendSV_IRQn, CORTEX_M3_INT_PEND_SV_PRI)

/* End of BSS marks the start of system stack. */
extern uint32_t _ebss;

/* Memory configuration. */
#define SYSTEM_STACK                    (((uint8_t *)&_ebss) + TARGET_HEAP_SIZE)
#define STM32F103_STACK_END             (0x20005000)
#define SYS_STACK_SIZE                  (STM32F103_STACK_END - (uint32_t)SYSTEM_STACK)

/* System registers. */
#define STM32_UUID                      ((uint8_t *)0x1FFFF7E8)

/* Function prototypes. */
uint64_t current_hardware_tick(void);

/* Helper functions. */
void system_entry(void);
void sysclock_init(void);
void wdt_disbale(void);

#endif /* _STM32F103_H_ */
