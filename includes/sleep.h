#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <os.h>

#ifdef CONFIG_INCLUDE_SLEEP

/* Scheduling class definition for sleeping tasks. */
extern SCHEDULER sleep_scheduler;

/* Function prototypes. */
void sleep_add_to_list(TASK *tcb, uint32_t ticks);
void sleep_remove_from_list(TASK *tcb);

#endif /* CONFIG_INCLUDE_SLEEP */

#endif /* _SLEEP_H_ */
