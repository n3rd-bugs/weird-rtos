/*
 * os_cortex_m3.h
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
#ifndef OS_CORTEX_M4_H
#define OS_CORTEX_M4_H

#include <os.h>

/* System interrupt level. */
extern uint32_t sys_interrupt_level;

/* Macros to manipulate interrupts. */
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
#define SET_INTERRUPT_LEVEL(n)          {                               \
                                            if (n == 0)                 \
                                            {                           \
                                                DISABLE_INTERRUPTS();   \
                                            }                           \
                                            else                        \
                                            {                           \
                                                ENABLE_INTERRUPTS();    \
                                            }                           \
                                        }

#define RESTORE_CONTEXT_FIRST()         run_first_task()
#define PEND_SV()                       {                                           \
                                            asm("   DSB    ");                      \
                                            asm("   ISB    ");                      \
                                            CORTEX_M4_PEND_SV_REG |= CORTEX_M4_PEND_SV_MAST;     \
                                            CORTEX_M4_SYS_TICK_REG &= ~(CORTEX_M4_SYS_TICK_MASK);\
                                            asm volatile                            \
                                            (                                       \
                                            "   MOVS        r0, #1          \r\n"   \
                                            "   MSR         BASEPRI, r0     \r\n"   \
                                            );                                      \
                                        }

#define CONTROL_TO_SYSTEM()             control_to_system()

#define RETURN_ENABLING_INTERRUPTS()    {                               \
                                            ENABLE_INTERRUPTS();        \
                                            asm("   BX      LR  ");     \
                                        }

#define INITIAL_XPSR                    0x01000000

/* Memory definitions. */
#define STATIC_MEM_START                ((char *)(&static_start))
#define STATIC_MEM_END                  ((char *)(&static_end))
#define DYNAMIC_MEM_START               ((char *)(&dynamic_start))
#define DYNAMIC_MEM_END                 ((char *)(&dynamic_end))

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
    uint32_t r14;
} software_stack_farme;

#define TOS_SET(tos, sp, size)      (tos = (sp + size))

/* Exported variables. */
extern uint32_t static_start;
extern uint32_t static_end;
extern uint32_t dynamic_start;
extern uint32_t dynamic_end;

/* Function prototypes. */
void run_first_task();
void control_to_system();

/* System interrupt definitions. */
ISR_FUN cpu_interrupt(void);
ISR_FUN nmi_interrupt(void);
ISR_FUN hard_fault_interrupt(void);
ISR_FUN isr_servicecall_handle(void);
NAKED_ISR_FUN isr_pendsv_handle(void);
ISR_FUN isr_sysclock_handle(void);
ISR_FUN isr_clock64_tick(void);

ISR_FUN usb_otg_interrupt(void);

#endif /* OS_CORTEX_M4_H */
