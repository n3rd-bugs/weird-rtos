/*
 * config.h
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
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Stack will be filled with this character. */
#define CONFIG_STACK_FILL                   'A'
#define CONFIG_PERIODIC_PIORITY             0
#define CONFIG_SLEEP_PIORITY                254
#define CONFIG_APERIODIC_PIORITY            255

/* Scheduler configuration. */
#define CONFIG_INCLUDE_TASK_STATS
#define CONFIG_INCLUDE_APERIODIC_TASKS
#define CONFIG_INCLUDE_PERIODIC_TASKS

/* Helper API configuration. */
#define CONFIG_INCLUDE_SLEEP
#define CONFIG_INCLUDE_SEMAPHORE

#endif /* _CONFIG_H_ */
