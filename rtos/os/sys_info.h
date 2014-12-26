/*
 * sys_info.h
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
#ifndef _SYS_INFO_H_
#define _SYS_INFO_H_

#include <config.h>

#ifdef CONFIG_INCLUDE_TASK_STATS

/* Function prototypes. */
uint32_t util_task_calc_free_stack(TASK *tcb);
void util_print_sys_info();

#endif /* CONFIG_INCLUDE_TASK_STATS */

#endif /* _SYS_INFO_H_ */
