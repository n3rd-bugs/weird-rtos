/*
 * os_avr.h
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

#ifndef OS_AVR_H
#define OS_AVR_H

#include <avr/io.h>
#include <tasks.h>

/* AVR specific configurations. */
#define OSC_FREQ            F_CPU
#define SYS_CLK_DIV         1
#define SYS_FREQ            (OSC_FREQ / SYS_CLK_DIV)

/* Macro to enable and disable interrupts. */
#define DISABLE_INTERRUPTS()    asm volatile ( "cli" :: );
#define ENABLE_INTERRUPTS()     asm volatile ( "sei" :: );

/* Critical section management. */
#define ENTRE_CRITICAL()    asm volatile ( "in      __tmp_reg__, __SREG__" :: );    \
                            asm volatile ( "cli" :: );                              \
                            asm volatile ( "push    __tmp_reg__" :: )

#define EXIT_CRITICAL()     asm volatile ( "pop     __tmp_reg__" :: );              \
                            asm volatile ( "out     __SREG__, __tmp_reg__" :: )

/* This macro saves a function's context on the stack. */
#define SAVE_CONTEXT()                                      \
    asm volatile (                                          \
                    "push   r0                      \n\t"   \
                    "in     r0, __SREG__            \n\t"   \
                    "cli                            \n\t"   \
                    "push   r0                      \n\t"   \
                    "push   r1                      \n\t"   \
                    "clr    r1                      \n\t"   \
                    "push   r2                      \n\t"   \
                    "push   r3                      \n\t"   \
                    "push   r4                      \n\t"   \
                    "push   r5                      \n\t"   \
                    "push   r6                      \n\t"   \
                    "push   r7                      \n\t"   \
                    "push   r8                      \n\t"   \
                    "push   r9                      \n\t"   \
                    "push   r10                     \n\t"   \
                    "push   r11                     \n\t"   \
                    "push   r12                     \n\t"   \
                    "push   r13                     \n\t"   \
                    "push   r14                     \n\t"   \
                    "push   r15                     \n\t"   \
                    "push   r16                     \n\t"   \
                    "push   r17                     \n\t"   \
                    "push   r18                     \n\t"   \
                    "push   r19                     \n\t"   \
                    "push   r20                     \n\t"   \
                    "push   r21                     \n\t"   \
                    "push   r22                     \n\t"   \
                    "push   r23                     \n\t"   \
                    "push   r24                     \n\t"   \
                    "push   r25                     \n\t"   \
                    "push   r26                     \n\t"   \
                    "push   r27                     \n\t"   \
                    "push   r28                     \n\t"   \
                    "push   r29                     \n\t"   \
                    "push   r30                     \n\t"   \
                    "push   r31                     \n\t"   \
                    "lds    r14, current_task        \n\t"       \
                    "lds    r15, current_task + 1    \n\t"       \
                    "movw   r26, r14                \n\t"   \
                    "ldi    r18, %[tos_offset]      \n\t"   \
                    "add    r26, r18                \n\t"   \
                    "adc    r27,__zero_reg__        \n\t"   \
                    "in     r0, 0x3d                \n\t"   \
                    "st     x+, r0                  \n\t"   \
                    "in     r0, 0x3e                \n\t"   \
                    "st     x+, r0                  \n\t"   \
                    :: [tos_offset] "M" (OFFSETOF(TASK, tos))    \
                  );

/* This macro loads a function's context from the stack. */
#define RESTORE_CONTEXT()                                   \
    asm volatile (                                          \
                    "lds    r14, current_task        \n\t"       \
                    "lds    r15, current_task + 1    \n\t"       \
                    "movw   r26, r14                \n\t"   \
                    "ldi    r18, %[tos_offset]      \n\t"   \
                    "add    r26, r18                \n\t"   \
                    "adc    r27,__zero_reg__        \n\t"   \
                    "ld     r28, x+                 \n\t"   \
                    "out    __SP_L__, r28           \n\t"   \
                    "ld     r29, x+                 \n\t"   \
                    "out    __SP_H__, r29           \n\t"   \
                    "pop    r31                     \n\t"   \
                    "pop    r30                     \n\t"   \
                    "pop    r29                     \n\t"   \
                    "pop    r28                     \n\t"   \
                    "pop    r27                     \n\t"   \
                    "pop    r26                     \n\t"   \
                    "pop    r25                     \n\t"   \
                    "pop    r24                     \n\t"   \
                    "pop    r23                     \n\t"   \
                    "pop    r22                     \n\t"   \
                    "pop    r21                     \n\t"   \
                    "pop    r20                     \n\t"   \
                    "pop    r19                     \n\t"   \
                    "pop    r18                     \n\t"   \
                    "pop    r17                     \n\t"   \
                    "pop    r16                     \n\t"   \
                    "pop    r15                     \n\t"   \
                    "pop    r14                     \n\t"   \
                    "pop    r13                     \n\t"   \
                    "pop    r12                     \n\t"   \
                    "pop    r11                     \n\t"   \
                    "pop    r10                     \n\t"   \
                    "pop    r9                      \n\t"   \
                    "pop    r8                      \n\t"   \
                    "pop    r7                      \n\t"   \
                    "pop    r6                      \n\t"   \
                    "pop    r5                      \n\t"   \
                    "pop    r4                      \n\t"   \
                    "pop    r3                      \n\t"   \
                    "pop    r2                      \n\t"   \
                    "pop    r1                      \n\t"   \
                    "pop    r0                      \n\t"   \
                    "out    __SREG__, r0            \n\t"   \
                    "pop    r0                      \n\t"   \
                    :: [tos_offset] "M" (OFFSETOF(TASK, tos))    \
                  );

/* This macro is responsible for switching context for time. */
#define RESTORE_CONTEXT_FIRST()         {                                   \
                                            RESTORE_CONTEXT();              \
                                            RETURN_ENABLING_INTERRUPTS();   \
                                        }

#define CONTROL_TO_SYSTEM()             control_to_system()

/* This macro tells the compiler to not manage the stack for a given function. */
#define STACK_LESS                      __attribute__ (( naked ))

/* Return statements for the functions which are stack less. */
#define RETURN_ENABLING_INTERRUPTS()    asm volatile ( "reti" )
#define RETURN_FUNCTION()               asm volatile ( "ret" )

#define TOS_SET(tos, sp, size)      (tos = (sp + (size-1)))

/* Function prototypes. */
void system_tick_Init();
void os_stack_init(TASK *tcb, TASK_ENTRY *entry, void *argv);
void control_to_system();

#endif /* OS_AVR_H */
