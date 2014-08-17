#ifndef _SYS_INFO_H_
#define _SYS_INFO_H_

#include <config.h>

#ifdef CONFIG_INCLUDE_TASK_STATS

/* Function prototypes. */
uint32_t util_task_calc_free_stack(TASK *tcb);
void util_print_sys_info();

#endif /* CONFIG_INCLUDE_TASK_STATS */

#endif /* _SYS_INFO_H_ */
