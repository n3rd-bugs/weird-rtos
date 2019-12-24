/*
 * cortex_m0.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _CORTEX_M0_H_
#define _CORTEX_M0_H_

#include <kernel.h>

/* Macros to manipulate interrupts. */
typedef uint8_t INT_LVL;
extern volatile INT_LVL sys_interrupt_level;
#define DISABLE_INTERRUPTS()            {                               \
                                            asm("   CPSID   I   ");     \
                                            sys_interrupt_level = 0;    \
                                        }
#define ENABLE_INTERRUPTS()             {                               \
                                            sys_interrupt_level = 1;    \
                                            asm("   DSB         ");     \
                                            asm("   ISB         ");     \
                                            asm("   CPSIE   I   ");     \
                                        }
#define GET_INTERRUPT_LEVEL()           (sys_interrupt_level)
#define SET_INTERRUPT_LEVEL(lvl)        {                               \
                                            if (lvl == 0)               \
                                            {                           \
                                                DISABLE_INTERRUPTS();   \
                                            }                           \
                                            else                        \
                                            {                           \
                                                ENABLE_INTERRUPTS();    \
                                            }                           \
                                        }

/* Scheduling macros. */
#define RESTORE_CONTEXT_FIRST()         run_first_task()
#define PEND_SV()                       CORTEX_M0_PEND_SV_REG |= CORTEX_M0_PEND_SV_MASK;
#define CONTROL_TO_SYSTEM()             control_to_system()

/* Memory definitions. */
#define STATIC_MEM_START                ((uint8_t *)(&static_start))
#define STATIC_MEM_END                  ((uint8_t *)(&static_end))
#define DYNAMIC_MEM_START               ((uint8_t *)(&dynamic_start))
#define DYNAMIC_MEM_END                 ((uint8_t *)(&dynamic_end))

/* Stack frame definitions. */
typedef struct _hardware_stack_farme
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} hardware_stack_farme;

typedef struct _software_stack_farme
{
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
} software_stack_farme;

/* Stack manipulation macros. */
#define TOS_SET(tos, sp, size)      (tos = (sp + size))

/* Exported variables. */
extern uint32_t static_start;
extern uint32_t static_end;
extern uint32_t dynamic_start;
extern uint32_t dynamic_end;

/* Function prototypes. */
void run_first_task(void);
void control_to_system(void);

/* System interrupt definitions. */
ISR_FUN cpu_interrupt(void);
ISR_FUN nmi_interrupt(void);
ISR_FUN hard_fault_interrupt(void);
ISR_FUN isr_servicecall_handle(void);
NAKED_ISR_FUN isr_sv_handle(void);
NAKED_ISR_FUN isr_pendsv_handle(void);
ISR_FUN isr_sysclock_handle(void);
ISR_FUN isr_clock64_tick(void);

#endif /* _CORTEX_M0_H_ */
