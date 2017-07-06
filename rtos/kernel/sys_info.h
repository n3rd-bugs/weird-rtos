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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _SYS_INFO_H_
#define _SYS_INFO_H_

#include <kernel.h>

#ifdef CONFIG_TASK_STATS
#ifdef CONFIG_FS
#include <fs.h>
#endif

/* Function prototypes. */
void usage_reset();
uint64_t usage_calculate(TASK *, uint64_t);
uint32_t util_task_calc_free_stack(TASK *);
void util_print_sys_info();
#ifdef SYS_STACK_SIZE
uint32_t util_system_calc_free_stack();
#endif
#ifdef CONFIG_SERIAL
void util_print_sys_info_assert();
#endif
#ifdef CONFIG_FS
int32_t util_print_sys_info_buffer(FS_BUFFER *);
#endif

#endif /* CONFIG_TASK_STATS */

#endif /* _SYS_INFO_H_ */
