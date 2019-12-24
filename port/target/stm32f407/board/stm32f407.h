/*
 * stm32f407.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

#ifndef _STM32F407_H_
#define _STM32F407_H_

#include <kernel.h>
#include <stm32f407xx.h>

/* Peripheral clock configuration. */
#define SYS_FREQ                        168000000
#define PCLK_FREQ                       84000000

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
#define STM32F407_STACK_END             (0x20020000)
#define SYS_STACK_SIZE                  (STM32F407_STACK_END - (uint32_t)SYSTEM_STACK)

/* System registers. */
#define STM32_UUID                      ((uint8_t *)0x1FFF7A10)

/* Function prototypes. */
uint64_t current_hardware_tick(void);

/* Helper functions. */
void system_entry(void);
void sysclock_init(void);
void wdt_disbale(void);

#endif /* _STM32F407_H_ */
