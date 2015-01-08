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
#include <os_target.h>
#include <tasks.h>
#include <scheduler.h>
#include <assert.h>
#ifdef CONFIG_SEMAPHORE
#include <semaphore.h>
#endif
#ifdef CONFIG_MEMGR
#include <mem.h>
#endif
#ifdef CONFIG_FS
#include <fs.h>
#endif

/* Some return codes. */
#define SUCCESS             0
#define FALSE               0
#define TRUE                1
#define MAX_WAIT            (-1)

#ifdef NULL
#undef NULL
#endif
#define NULL                0

/* Number of system ticks per second. */
#define OS_TICKS_PER_SEC    100
#define OS_TICK64_PER_SEC   1000000

/* Some useful macros. */
#define OFFSETOF(type, field)   ((int) &(((type *) 0)->field))
#define UNUSED_PARAM(x)         (void)(x)

/* Defines the origin from which this task is being yielded.  */
#define YIELD_INIT              0x00
#define YIELD_SYSTEM            0x01
#define YIELD_MANUAL            0x02
#define YIELD_CANNOT_RUN        0x03

/* Alignment manipulation macros. */
#define ALLIGN_SIZE             (0x4)
#define ALLIGN_FLOOR(n)         (((n) % ALLIGN_SIZE) ? ((n) & ~(0x3)) : (n))
#define ALLIGN_CEIL(n)          (((n) % ALLIGN_SIZE) ? ((n) & ~(0x3)) + ALLIGN_SIZE : (n))

/* Exported variables. */
extern TASK *current_task;

/* ISR routines. */
#define OS_ISR_ENTER();         TASK *task_save = current_task;         \
                                current_task = NULL;


#define OS_ISR_EXIT();          current_task = task_save;

/* Public function prototypes. */
void os_run();
void task_yield();

/* External function prototypes. */
void sleep(uint32_t ticks);
#define sleep_ms(ms)            sleep( (ms * OS_TICKS_PER_SEC) / (1000) )

/* Internal functions should not be called from user applications. */
void os_process_system_tick();
void task_waiting();

void set_current_task(TASK *tcb);
TASK *get_current_task();
uint64_t current_system_tick();

#endif /* _OS_H_ */
