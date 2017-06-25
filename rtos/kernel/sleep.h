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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <os.h>

#ifdef CONFIG_SLEEP

/* Function prototypes. */
void sleep_process_system_tick();
void sleep_add_to_list(TASK *, uint32_t);
void sleep_remove_from_list(TASK *);

#endif /* CONFIG_SLEEP */

#endif /* _SLEEP_H_ */
