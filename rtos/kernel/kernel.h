/*
 * kernel.h
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
#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdint.h>
#include <config.h>
#include <tasks.h>
#include <scheduler.h>
#include <assert.h>
#include <kernel_target.h>
#include <sys_log.h>
#include <sleep.h>

/* Some return codes. */
#define SUCCESS                     (0)
#define FALSE                       (0)
#define TRUE                        (1)
#define PARTIAL                     (2)
#define MAX_WAIT                    (uint32_t)(-1)

#ifdef NULL
#undef NULL
#endif
#define NULL                        (0)

/* Number of system ticks per second. */
#define SOFT_TICKS_PER_SEC          (uint32_t)(100)
#define MS_TO_TICK(a)               ((uint32_t)(((uint32_t)(a) * SOFT_TICKS_PER_SEC) / (1000)))
#define TICK_TO_MS(a)               ((uint32_t)(((uint32_t)(a) * 1000) / (SOFT_TICKS_PER_SEC)))
#define US_TO_HW_TICK(a)            ((uint32_t)(((uint32_t)(a) * HW_TICKS_PER_SEC) / (1000000)))
#define HW_TICK_TO_US(a)            ((uint32_t)(((uint32_t)(a) * 1000000) / (HW_TICKS_PER_SEC)))

/* Some useful macros. */
#define OFFSETOF(type, field)       ((int) &(((type *) 0)->field))
#define UNUSED_PARAM(x)             (void)(x)
#define MASK_N_BITS(x)              ((uint32_t)((1 << x) - 1))
#define READ_REG32(x)               (*(uint32_t *)x)
#define WRITE_REG32(x, v)           (*(uint32_t *)x = v)
#define MASK_REG32(x, clr, set)     WRITE_REG32(x, (((READ_REG32(x)) & (uint32_t)(~clr)) | set))
#define MIN(a, b)                   (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                   (((a) > (b)) ? (a) : (b))
#define INT32CMP(a, b)              ((int32_t)((a) - (b)))

/* Alignment manipulation macros. */
#define ALLIGN_SIZE                 (uint32_t)(0x8)
#define ALLIGN_FLOOR_N(n, num)      (uint32_t)(((n) % num) ? ((n) & (uint32_t)~(num - 1)) : (n))
#define ALLIGN_CEIL_N(n, num)       (uint32_t)(((n) % num) ? ((n) & (uint32_t)~(num - 1)) + num : (n))
#define ALLIGN_FLOOR(n)             (uint32_t)(((n) % ALLIGN_SIZE) ? ((n) & (uint32_t)~(ALLIGN_SIZE - 1)) : (n))
#define ALLIGN_CEIL(n)              (uint32_t)(((n) % ALLIGN_SIZE) ? ((n) & (uint32_t)~(ALLIGN_SIZE - 1)) + ALLIGN_SIZE : (n))

/* Exported definitions. */
extern TASK *return_task;
extern TASK *current_task;
extern uint32_t current_tick;

/* ISR routines. */
#ifndef CPU_ISR_ENTER
#define CPU_ISR_ENTER()
#endif
#ifndef CPU_ISR_EXIT
#define CPU_ISR_EXIT()
#endif

#if (defined(CONFIG_TASK_STATS) && defined(CONFIG_TASK_USAGE))
#define MARK_ENTRY()                mark_task_entry()
#define MARK_EXIT()                 mark_task_exit()
#else
#define MARK_ENTRY()
#define MARK_EXIT()
#endif /*  (defined(CONFIG_TASK_STATS) && defined(CONFIG_TASK_USAGE)) */

#define ISR_ENTER()                 CPU_ISR_ENTER();                            \
                                    return_task = get_current_task();           \
                                    MARK_EXIT();                                \
                                    set_current_task(NULL)

#define ISR_EXIT()                  set_current_task(return_task);              \
                                    MARK_ENTRY();                               \
                                    return_task = NULL;                         \
                                    CPU_ISR_EXIT()

#define KERNEL_RUNNING()            ((current_task != NULL) || (return_task != NULL))

/* Kernel macro definitions. */
#define get_current_task()          (current_task)
#define set_current_task(task)      current_task = task
#define process_system_tick()       current_tick ++

/* Public function prototypes. */
void kernel_run();
void task_yield();
uint32_t current_system_tick();

#if (defined(CONFIG_TASK_STATS) && defined(CONFIG_TASK_USAGE))
void mark_task_entry();
void mark_task_exit();
#endif /*  (defined(CONFIG_TASK_STATS) && defined(CONFIG_TASK_USAGE)) */

#endif /* _KERNEL_H_ */
