/*
 * kernel.h
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
#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdint.h>
#include <config.h>
#include <kernel_target.h>
#include <tasks.h>
#include <scheduler.h>
#include <assert.h>
#include <sys_log.h>
#include <sleep.h>
#include <kernel_config.h>

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

/* Tick to time conversion macros. */
#define MS_TO_TICK(a)               ((uint32_t)(((uint32_t)(a) * SOFT_TICKS_PER_SEC) / (1000)))
#define TICK_TO_MS(a)               ((uint32_t)(((uint32_t)(a) * 1000) / (SOFT_TICKS_PER_SEC)))
#define US_TO_HW_TICK(a)            ((uint64_t)(((uint64_t)(a) * HW_TICKS_PER_SEC) / (1000000)))
#define HW_TICK_TO_US(a)            ((uint64_t)(((uint64_t)(a) * 1000000) / (HW_TICKS_PER_SEC)))

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
#define INT64CMP(a, b)              ((int64_t)((a) - (b)))
#define CEIL_DIV(a, b)              (((a) + (b) - 1) / (b))

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
#ifndef CPU_ISR_RETURN
#define CPU_ISR_RETURN()            CPU_ISR_ENTER();                            \
                                    CPU_ISR_EXIT();
#endif

#if (defined(TASK_STATS) && defined(TASK_USAGE))
#define MARK_ENTRY()                mark_task_entry()
#define MARK_EXIT()                 mark_task_exit()
#else
#define MARK_ENTRY()
#define MARK_EXIT()
#endif /*  (defined(TASK_STATS) && defined(TASK_USAGE)) */

#define ISR_ENTER()                 CPU_ISR_ENTER();                            \
                                    return_task = get_current_task();           \
                                    MARK_EXIT();                                \
                                    set_current_task(NULL)

#define ISR_EXIT()                  set_current_task(return_task);              \
                                    MARK_ENTRY();                               \
                                    return_task = NULL;                         \
                                    CPU_ISR_EXIT()

#define ISR_RETURN()                CPU_ISR_RETURN()

#define KERNEL_RUNNING()            ((current_task != NULL) || (return_task != NULL))

/* Kernel macro definitions. */
#define get_current_task(void)      (current_task)
#define set_current_task(task)      current_task = task

/* Public function prototypes. */
void kernel_run(void);
void task_yield(void);

#if (defined(TASK_STATS) && defined(TASK_USAGE))
void mark_task_entry(void);
void mark_task_exit(void);
#endif /*  (defined(TASK_STATS) && defined(TASK_USAGE)) */

#endif /* _KERNEL_H_ */
