/*
 * os.h
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
#ifndef _OS_H_
#define _OS_H_

#include <stdint.h>
#include <config.h>
#include <tasks.h>
#include <scheduler.h>
#include <assert.h>
#include <os_target.h>

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
#define OS_TICKS_PER_SEC            (uint64_t)(100)
#define MS_TO_TICK(a)               ((uint64_t)(((uint64_t)(a) * OS_TICKS_PER_SEC) / (1000)))
#define TICK_TO_MS(a)               ((uint64_t)(((uint64_t)(a) * 1000) / (OS_TICKS_PER_SEC)))
#define US_TO_HW_TICK(a)            ((uint64_t)(((uint64_t)(a) * OS_HW_TICKS_PER_SEC) / (1000000)))
#define HW_TICK_TO_US(a)            ((uint64_t)(((uint64_t)(a) * 1000000) / (OS_HW_TICKS_PER_SEC)))

/* Some useful macros. */
#define OFFSETOF(type, field)       ((int) &(((type *) 0)->field))
#define UNUSED_PARAM(x)             (void)(x)
#define MASK_N_BITS(x)              ((uint32_t)((1 << x) - 1))
#define OS_READ_REG32(x)            (*(uint32_t *)x)
#define OS_WRITE_REG32(x, v)        (*(uint32_t *)x = v)
#define OS_MASK_REG32(x, clr, set)  OS_WRITE_REG32(x, (((OS_READ_REG32(x)) & (uint32_t)(~clr)) | set))
#define MIN(a, b)                   (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                   (((a) > (b)) ? (a) : (b))

/* Alignment manipulation macros. */
#define ALLIGN_SIZE                 (uint32_t)(0x4)
#define ALLIGN_FLOOR_N(n, num)      (uint32_t)(((n) % num) ? ((n) & (uint32_t)~(num - 1)) : (n))
#define ALLIGN_CEIL_N(n, num)       (uint32_t)(((n) % num) ? ((n) & (uint32_t)~(num - 1)) + num : (n))
#define ALLIGN_FLOOR(n)             (uint32_t)(((n) % ALLIGN_SIZE) ? ((n) & (uint32_t)~(ALLIGN_SIZE - 1)) : (n))
#define ALLIGN_CEIL(n)              (uint32_t)(((n) % ALLIGN_SIZE) ? ((n) & (uint32_t)~(ALLIGN_SIZE - 1)) + ALLIGN_SIZE : (n))

/* ISR routines. */
extern TASK *return_task;
#ifndef CPU_ISR_ENTER
#define CPU_ISR_ENTER()
#endif
#ifndef CPU_ISR_EXIT
#define CPU_ISR_EXIT()
#endif

#define OS_ISR_ENTER();                                                         \
                                    CPU_ISR_ENTER();                            \
                                    return_task = get_current_task();           \
                                    set_current_task(NULL);

#define OS_ISR_EXIT();              set_current_task(return_task);              \
                                    return_task = NULL;                         \
                                    CPU_ISR_EXIT();

/* Public function prototypes. */
void os_run();
void task_yield();

/* External function prototypes. */
void sleep_ticks(uint32_t);
void sleep_hw_ticks(uint64_t);
#define sleep_ms(ms)                sleep_ticks(MS_TO_TICK((ms)))
#define sleep_us(us)                sleep_hw_ticks(US_TO_HW_TICK((us)))

/* Internal functions should not be called from user applications. */
void os_process_system_tick();

void set_current_task(TASK *);
TASK *get_current_task();
uint64_t current_system_tick();

#endif /* _OS_H_ */
