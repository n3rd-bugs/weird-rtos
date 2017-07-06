/*
 * sleep.h
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
#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <kernel.h>

#ifdef CONFIG_SLEEP

/* Function prototypes. */
void sleep_process_system_tick();
void sleep_add_to_list(TASK *, uint32_t);
void sleep_remove_from_list(TASK *);
void sleep_ticks(uint32_t);
void sleep_hw_ticks(uint64_t);
#define sleep_ms(ms)                sleep_ticks(MS_TO_TICK((ms)))
#define sleep_us(us)                sleep_hw_ticks(US_TO_HW_TICK((us)))

#endif /* CONFIG_SLEEP */

#endif /* _SLEEP_H_ */
