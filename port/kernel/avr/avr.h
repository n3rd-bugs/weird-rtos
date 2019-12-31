/*
 * avr.h
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
#ifndef _AVR_H_
#define _AVR_H_

#include <avr/io.h>
#include <tasks.h>

/* AVR specific configurations. */
#ifndef F_CPU
#define F_CPU 20000000
#endif
#define OSC_FREQ                F_CPU
#define SYS_CLK_DIV             1
#define SYS_FREQ                (OSC_FREQ / SYS_CLK_DIV)
#define PCLK_FREQ               SYS_FREQ
#define AVR_HARD_RESET          FALSE
#define HW_TICKS_PER_SEC        (PCLK_FREQ / 64)
#define SYSTEM_STACK            (((uint8_t *)&__heap_start) + TARGET_HEAP_SIZE)
#define SYS_STACK_SIZE          (system_stack_end - (uint16_t)SYSTEM_STACK)

/* System interrupt level. */
extern volatile uint8_t sys_interrupt_level;

/* System stack definitions. */
extern uint8_t __heap_start;
extern uint16_t system_stack_end;

/* Flag to specify that we are in ISR context. */
extern uint8_t avr_in_isr;

/* Macros to manipulate interrupts. */
typedef uint8_t INT_LVL;
#define ENABLE_INTERRUPTS()                                             \
    {                                                                   \
        sys_interrupt_level = 1;                                        \
        if (return_task == NULL)                                        \
        {                                                               \
            asm volatile("   SEI         ");                            \
        }                                                               \
    }

#define DISABLE_INTERRUPTS()                                            \
    {                                                                   \
        asm volatile("   CLI         ");                                \
        sys_interrupt_level = 0;                                        \
    }

#define GET_INTERRUPT_LEVEL()                                           \
    (sys_interrupt_level)
#define SET_INTERRUPT_LEVEL(n)                                          \
    {                                                                   \
        if (n == 0)                                                     \
        {                                                               \
            DISABLE_INTERRUPTS();                                       \
        }                                                               \
        else                                                            \
        {                                                               \
            ENABLE_INTERRUPTS();                                        \
        }                                                               \
    }

/* Critical section management. */
#define ENTRE_CRITICAL()                                                \
    asm volatile (                                                      \
                    "IN      __tmp_reg__, __SREG__              \r\n"   \
                    "CLI                                        \r\n"   \
                    "PUSH    __tmp_reg__                        \r\n"   \
                  );

#define EXIT_CRITICAL()                                                 \
    asm volatile (                                                      \
                    "POP     __tmp_reg__                        \r\n"   \
                    "OUT     __SREG__, __tmp_reg__              \r\n"   \
                  );

#define WDT_RESET()                                                     \
    asm volatile ( "    WDR         ");

#define CPU_ISR_RETURN()                                                \
    asm volatile ( "    RETI        " );

#define CPU_ISR_ENTER()                                                 \
    {                                                                   \
        SAVE_CONTEXT_ISR();                                             \
        LOAD_SYSTEM_STACK();                                            \
        avr_in_isr = TRUE;                                              \
        sys_interrupt_level = 0;                                        \
    }

#define CPU_ISR_EXIT()                                                  \
    {                                                                   \
        avr_in_isr = FALSE;                                             \
        RESTORE_CONTEXT();                                              \
    }

/* Load system stack. */
#define LOAD_SYSTEM_STACK()                                             \
    asm volatile (                                                      \
                    "LDS   R28,        system_stack_end         \r\n"   \
                    "LDS   R29,        system_stack_end + 1     \r\n"   \
                    "OUT   __SP_L__,   R28                      \r\n"   \
                    "OUT   __SP_H__,   R29                      \r\n"   \
                   );

/* This macro saves either a task's or an ISR's context on the stack. */
#define SAVE_CONTEXT_CTS()                                              \
    asm volatile (                                                      \
                    "PUSH   R16                                 \r\n"   \
                    "LDS    R16,        avr_in_isr              \r\n"   \
                    "SBRS   R16,        0                       \r\n"   \
                    "RJMP   __save_task_                        \r\n"   \
                    "PUSH   R1                                  \r\n"   \
                    "EOR    R1,         R1                      \r\n"   \
                    "PUSH   R2                                  \r\n"   \
                    "PUSH   R3                                  \r\n"   \
                    "PUSH   R4                                  \r\n"   \
                    "PUSH   R5                                  \r\n"   \
                    "PUSH   R6                                  \r\n"   \
                    "PUSH   R7                                  \r\n"   \
                    "PUSH   R8                                  \r\n"   \
                    "PUSH   R9                                  \r\n"   \
                    "PUSH   R10                                 \r\n"   \
                    "PUSH   R11                                 \r\n"   \
                    "PUSH   R12                                 \r\n"   \
                    "PUSH   R13                                 \r\n"   \
                    "PUSH   R14                                 \r\n"   \
                    "PUSH   R15                                 \r\n"   \
                    "PUSH   R0                                  \r\n"   \
                    "PUSH   R17                                 \r\n"   \
                    "PUSH   R18                                 \r\n"   \
                    "PUSH   R19                                 \r\n"   \
                    "PUSH   R20                                 \r\n"   \
                    "PUSH   R21                                 \r\n"   \
                    "PUSH   R22                                 \r\n"   \
                    "PUSH   R23                                 \r\n"   \
                    "PUSH   R24                                 \r\n"   \
                    "PUSH   R25                                 \r\n"   \
                    "PUSH   R26                                 \r\n"   \
                    "PUSH   R27                                 \r\n"   \
                    "PUSH   R28                                 \r\n"   \
                    "PUSH   R29                                 \r\n"   \
                    "PUSH   R30                                 \r\n"   \
                    "PUSH   R31                                 \r\n"   \
                    "RJMP   __skip_save_task_                   \r\n"   \
                    "__save_task_:                              \r\n"   \
                    "IN     R16,        __SREG__                \r\n"   \
                    "CLI                                        \r\n"   \
                    "PUSH   R16                                 \r\n"   \
                    "IN     R16,        0x3B                    \r\n"   \
                    "PUSH   R16                                 \r\n"   \
                    "PUSH   R1                                  \r\n"   \
                    "EOR    R1,         R1                      \r\n"   \
                    "PUSH   R2                                  \r\n"   \
                    "PUSH   R3                                  \r\n"   \
                    "PUSH   R4                                  \r\n"   \
                    "PUSH   R5                                  \r\n"   \
                    "PUSH   R6                                  \r\n"   \
                    "PUSH   R7                                  \r\n"   \
                    "PUSH   R8                                  \r\n"   \
                    "PUSH   R9                                  \r\n"   \
                    "PUSH   R10                                 \r\n"   \
                    "PUSH   R11                                 \r\n"   \
                    "PUSH   R12                                 \r\n"   \
                    "PUSH   R13                                 \r\n"   \
                    "PUSH   R14                                 \r\n"   \
                    "PUSH   R15                                 \r\n"   \
                    "PUSH   R0                                  \r\n"   \
                    "PUSH   R17                                 \r\n"   \
                    "PUSH   R18                                 \r\n"   \
                    "PUSH   R19                                 \r\n"   \
                    "PUSH   R20                                 \r\n"   \
                    "PUSH   R21                                 \r\n"   \
                    "PUSH   R22                                 \r\n"   \
                    "PUSH   R23                                 \r\n"   \
                    "PUSH   R24                                 \r\n"   \
                    "PUSH   R25                                 \r\n"   \
                    "PUSH   R26                                 \r\n"   \
                    "PUSH   R27                                 \r\n"   \
                    "PUSH   R28                                 \r\n"   \
                    "PUSH   R29                                 \r\n"   \
                    "PUSH   R30                                 \r\n"   \
                    "PUSH   R31                                 \r\n"   \
                    "LDS    R14,        sys_interrupt_level     \r\n"   \
                    "PUSH   R14                                 \r\n"   \
                    "LDS    R14,        current_task            \r\n"   \
                    "LDS    R15,        current_task + 1        \r\n"   \
                    "MOVW   R26,        R14                     \r\n"   \
                    "LDI    R18,        %[tos_offset]           \r\n"   \
                    "ADD    R26,        R18                     \r\n"   \
                    "ADC    R27,        __zero_reg__            \r\n"   \
                    "IN     R0,         __SP_L__                \r\n"   \
                    "ST     X+,         R0                      \r\n"   \
                    "IN     R0,         __SP_H__                \r\n"   \
                    "ST     X+,         R0                      \r\n"   \
                    "LDS    R28,        system_stack_end        \r\n"   \
                    "LDS    R29,        system_stack_end + 1    \r\n"   \
                    "OUT    __SP_L__,   R28                     \r\n"   \
                    "OUT    __SP_H__,   R29                     \r\n"   \
                    "__skip_save_task_:                         \r\n"   \
                    :: [tos_offset] "M" (OFFSETOF(TASK, tos))           \
                  );

/* This macro saves a task's context on the stack and saves the SREG after
 * setting interrupt bit. */
#define SAVE_CONTEXT_ISR()                                              \
    asm volatile (                                                      \
                    "CLI                                        \r\n"   \
                    "PUSH   R16                                 \r\n"   \
                    "IN     R16,        __SREG__                \r\n"   \
                    "SBR    R16,        0x80                    \r\n"   \
                    "PUSH   R16                                 \r\n"   \
                    "IN     R16,        0x3B                    \r\n"   \
                    "PUSH   R16                                 \r\n"   \
                    "PUSH   R1                                  \r\n"   \
                    "EOR    R1,         R1                      \r\n"   \
                    "PUSH   R2                                  \r\n"   \
                    "PUSH   R3                                  \r\n"   \
                    "PUSH   R4                                  \r\n"   \
                    "PUSH   R5                                  \r\n"   \
                    "PUSH   R6                                  \r\n"   \
                    "PUSH   R7                                  \r\n"   \
                    "PUSH   R8                                  \r\n"   \
                    "PUSH   R9                                  \r\n"   \
                    "PUSH   R10                                 \r\n"   \
                    "PUSH   R11                                 \r\n"   \
                    "PUSH   R12                                 \r\n"   \
                    "PUSH   R13                                 \r\n"   \
                    "PUSH   R14                                 \r\n"   \
                    "PUSH   R15                                 \r\n"   \
                    "PUSH   R0                                  \r\n"   \
                    "PUSH   R17                                 \r\n"   \
                    "PUSH   R18                                 \r\n"   \
                    "PUSH   R19                                 \r\n"   \
                    "PUSH   R20                                 \r\n"   \
                    "PUSH   R21                                 \r\n"   \
                    "PUSH   R22                                 \r\n"   \
                    "PUSH   R23                                 \r\n"   \
                    "PUSH   R24                                 \r\n"   \
                    "PUSH   R25                                 \r\n"   \
                    "PUSH   R26                                 \r\n"   \
                    "PUSH   R27                                 \r\n"   \
                    "PUSH   R28                                 \r\n"   \
                    "PUSH   R29                                 \r\n"   \
                    "PUSH   R30                                 \r\n"   \
                    "PUSH   R31                                 \r\n"   \
                    "LDS    R14,        sys_interrupt_level     \r\n"   \
                    "PUSH   R14                                 \r\n"   \
                    "LDS    R14,        current_task            \r\n"   \
                    "LDS    R15,        current_task + 1        \r\n"   \
                    "MOVW   R26,        R14                     \r\n"   \
                    "LDI    R18,        %[tos_offset]           \r\n"   \
                    "ADD    R26,        R18                     \r\n"   \
                    "ADC    R27,        __zero_reg__            \r\n"   \
                    "IN     R0,         __SP_L__                \r\n"   \
                    "ST     X+,         R0                      \r\n"   \
                    "IN     R0,         __SP_H__                \r\n"   \
                    "ST     X+,         R0                      \r\n"   \
                    :: [tos_offset] "M" (OFFSETOF(TASK, tos))           \
                  );

/* This macro loads a task's context from the stack. */
#define RESTORE_CONTEXT()                                               \
    asm volatile (                                                      \
                    "LDS    R14,        current_task            \r\n"   \
                    "LDS    R15,        current_task + 1        \r\n"   \
                    "MOVW   R26,        R14                     \r\n"   \
                    "LDI    R18,        %[tos_offset]           \r\n"   \
                    "ADD    R26,        R18                     \r\n"   \
                    "ADC    R27,        __zero_reg__            \r\n"   \
                    "LD     R28,        X+                      \r\n"   \
                    "OUT    __SP_L__,   R28                     \r\n"   \
                    "LD     R29, X+                             \r\n"   \
                    "OUT    __SP_H__,   R29                     \r\n"   \
                    "POP    R14                                 \r\n"   \
                    "STS    sys_interrupt_level, R14            \r\n"   \
                    "POP    R31                                 \r\n"   \
                    "POP    R30                                 \r\n"   \
                    "POP    R29                                 \r\n"   \
                    "POP    R28                                 \r\n"   \
                    "POP    R27                                 \r\n"   \
                    "POP    R26                                 \r\n"   \
                    "POP    R25                                 \r\n"   \
                    "POP    R24                                 \r\n"   \
                    "POP    R23                                 \r\n"   \
                    "POP    R22                                 \r\n"   \
                    "POP    R21                                 \r\n"   \
                    "POP    R20                                 \r\n"   \
                    "POP    R19                                 \r\n"   \
                    "POP    R18                                 \r\n"   \
                    "POP    R17                                 \r\n"   \
                    "POP    R0                                  \r\n"   \
                    "POP    R15                                 \r\n"   \
                    "POP    R14                                 \r\n"   \
                    "POP    R13                                 \r\n"   \
                    "POP    R12                                 \r\n"   \
                    "POP    R11                                 \r\n"   \
                    "POP    R10                                 \r\n"   \
                    "POP    R9                                  \r\n"   \
                    "POP    R8                                  \r\n"   \
                    "POP    R7                                  \r\n"   \
                    "POP    R6                                  \r\n"   \
                    "POP    R5                                  \r\n"   \
                    "POP    R4                                  \r\n"   \
                    "POP    R3                                  \r\n"   \
                    "POP    R2                                  \r\n"   \
                    "POP    R1                                  \r\n"   \
                    "POP    R16                                 \r\n"   \
                    "OUT    0x3B,       R16                     \r\n"   \
                    "POP    R16                                 \r\n"   \
                    "SBRS   R16,        0x7                     \r\n"   \
                    "RJMP   .+8                                 \r\n"   \
                    "CBR    R16,        0x80                    \r\n"   \
                    "OUT    __SREG__,   R16                     \r\n"   \
                    "POP    R16                                 \r\n"   \
                    "RETI                                       \r\n"   \
                    "OUT    __SREG__,   R16                     \r\n"   \
                    "POP    R16                                 \r\n"   \
                    "RET                                        \r\n"   \
                    :: [tos_offset] "M" (OFFSETOF(TASK, tos))           \
                 );

/* This macro loads a function's context from the stack. */
#define RESTORE_STACK()                                                 \
    asm volatile (                                                      \
                    "POP    R31                                 \r\n"   \
                    "POP    R30                                 \r\n"   \
                    "POP    R29                                 \r\n"   \
                    "POP    R28                                 \r\n"   \
                    "POP    R27                                 \r\n"   \
                    "POP    R26                                 \r\n"   \
                    "POP    R25                                 \r\n"   \
                    "POP    R24                                 \r\n"   \
                    "POP    R23                                 \r\n"   \
                    "POP    R22                                 \r\n"   \
                    "POP    R21                                 \r\n"   \
                    "POP    R20                                 \r\n"   \
                    "POP    R19                                 \r\n"   \
                    "POP    R18                                 \r\n"   \
                    "POP    R17                                 \r\n"   \
                    "POP    R0                                  \r\n"   \
                    "POP    R15                                 \r\n"   \
                    "POP    R14                                 \r\n"   \
                    "POP    R13                                 \r\n"   \
                    "POP    R12                                 \r\n"   \
                    "POP    R11                                 \r\n"   \
                    "POP    R10                                 \r\n"   \
                    "POP    R9                                  \r\n"   \
                    "POP    R8                                  \r\n"   \
                    "POP    R7                                  \r\n"   \
                    "POP    R6                                  \r\n"   \
                    "POP    R5                                  \r\n"   \
                    "POP    R4                                  \r\n"   \
                    "POP    R3                                  \r\n"   \
                    "POP    R2                                  \r\n"   \
                    "POP    R1                                  \r\n"   \
                    "POP    R16                                 \r\n"   \
                );

/* This macro is responsible for switching context for time. */
#define RESTORE_CONTEXT_FIRST()                                         \
    {                                                                   \
        current_task = scheduler_get_next_task();                       \
        current_task->state = TASK_RUNNING;                             \
        MARK_ENTRY();                                                   \
        sys_interrupt_level = TRUE;                                     \
        RESTORE_CONTEXT();                                              \
    }

#define CONTROL_TO_SYSTEM()                                             \
    control_to_system()

/* Return statements for the functions which are stack less. */
#define RETURN_ENABLING_INTERRUPTS()                                    \
    asm volatile ( "RETI" )
#define RETURN_FUNCTION()                                               \
    asm volatile ( "RET" )

#define TOS_SET(tos, sp, size)                                          \
    (tos = (sp + (size-1)))

/* Function prototypes. */
void stack_init(TASK *tcb, TASK_ENTRY *entry, void *argv);
NAKED_FUN control_to_system(void);

#endif /* _AVR_H_ */
