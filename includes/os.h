/*
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose.
 */
#ifndef _OS_H_
#define _OS_H_

#include <avr/io.h>
#include <config.h>
#include <os_avr.h>
#include <tasks.h>
#include <scheduler.h>
#include <semaphore.h>

/* Some return codes. */
#define SUCCESS             0
#define FALSE               0
#define TRUE                1

#ifndef NULL
#define NULL                0
#endif

/* Number of system ticks per second. */
#define OS_TICKS_PER_SEC    1000

/* Some useful macros. */
#define OFFSETOF(type, field)   ((uint16_t) &(((type *) 0)->field))
#define UNUSED_PARAM(x)         (void)(x)

/* Defines the origin from which this task is being yielded.  */
#define YIELD_INIT              0x00
#define YIELD_SYSTEM            0x01
#define YIELD_MANUAL            0x02
#define YIELD_CANNOT_RUN        0x03

/* Public function prototypes. */
void os_run();
uint64_t current_system_tick();

/* External function prototypes. */
void sleep(uint32_t ticks);

/* Internal functions should not be called from user applications. */
void os_process_system_tick()   STACK_LESS;
void task_yield()               STACK_LESS;
void task_waiting()             STACK_LESS;

void set_current_task(TASK *tcb);
TASK *get_current_task();
uint64_t current_system_tick();

#endif /* _OS_H_ */
