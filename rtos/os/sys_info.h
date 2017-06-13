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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#ifndef _SYS_INFO_H_
#define _SYS_INFO_H_

#include <os.h>

#ifdef CONFIG_TASK_STATS
#ifdef CONFIG_FS
#include <fs.h>
#endif

/* Function prototypes. */
uint32_t util_task_calc_free_stack(TASK *);
void util_print_sys_info();
#ifdef CONFIG_FS
int32_t util_print_sys_info_buffer(FS_BUFFER *);
#endif

#endif /* CONFIG_TASK_STATS */

#endif /* _SYS_INFO_H_ */
