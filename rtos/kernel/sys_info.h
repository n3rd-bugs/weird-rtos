/*
 * sys_info.h
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
#ifndef _SYS_INFO_H_
#define _SYS_INFO_H_
#include <kernel.h>

#ifdef TASK_STATS
#ifdef CONFIG_FS
#include <fs.h>
#endif

/* Function prototypes. */
void usage_reset(void);
uint64_t usage_calculate(TASK *, uint64_t);
uint32_t util_task_calc_free_stack(TASK *);
#ifdef CONFIG_SERIAL
void util_print_sys_info(void);
#endif /* CONFIG_SERIAL */
#ifdef SYS_STACK_SIZE
uint32_t util_system_calc_free_stack(void);
#endif /* SYS_STACK_SIZE */
#ifdef CONFIG_SERIAL
void util_print_sys_info_assert(void);
#endif /* CONFIG_SERIAL */
#ifdef CONFIG_FS
int32_t util_print_sys_info_buffer(FS_BUFFER_LIST *);
#endif /* CONFIG_FS */

#endif /* TASK_STATS */
#endif /* _SYS_INFO_H_ */
