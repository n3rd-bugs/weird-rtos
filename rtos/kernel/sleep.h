/*
 * sleep.h
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
#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <kernel.h>

#ifdef CONFIG_SLEEP

/* Function prototypes. */
void sleep_process_system_tick(void);
void sleep_add_to_list(TASK *, uint32_t);
void sleep_remove_from_list(TASK *);
void sleep_ticks(uint32_t);
void sleep_hw_ticks(uint64_t);
#define sleep_ms(ms)                sleep_ticks(MS_TO_TICK((ms)))
#define sleep_fms(ms)               sleep_ticks((MS_TO_TICK((ms)) > 0) ? MS_TO_TICK((ms)) : 1)
#define sleep_us(us)                sleep_hw_ticks(US_TO_HW_TICK((us)))

/* Kernel APIs. */
uint32_t current_system_tick(void);
uint8_t process_system_tick(void);

#endif /* CONFIG_SLEEP */

#endif /* _SLEEP_H_ */
