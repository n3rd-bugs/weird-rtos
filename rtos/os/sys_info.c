/*
 * sys_info.c
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
#include <os.h>
#include <sys_info.h>
#include <serial.h>

#ifdef CONFIG_TASK_STATS

/*
 * util_task_calc_free_stack
 * @tcb: Task control block of which number of free bytes on the stack are
 *  needed to be calculated.
 * @return: Number of bytes free on the given task's stack.
 * This function calculate and returns number of bytes free on a given task's
 * stack.
 */
uint32_t util_task_calc_free_stack(TASK *tcb)
{
    uint32_t free = 0;

    /* Calculate the number of bytes intact on this task's
     * stack. */
    while (tcb->stack_start[free] == CONFIG_STACK_PATTERN)
    {
        free ++;
    }

    /* Return number of free bytes on the stack. */
    return (free);

} /* util_task_calc_free_stack */

/*
 * util_print_sys_info
 * This function prints generalized information about the operating system.
 */
void util_print_sys_info()
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_used;

    /* Print current system tick. */
    printf("System tick: %lu\r\n", (uint32_t)current_system_tick());

    /* Print table header. */
    printf("Name\tClass\tTotal\tFree\tMin.\tn(T)\r\n");

    /* Print information about all the tasks in the system. */
    while (tcb != NULL)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_used = util_task_calc_free_stack(tcb);

        /* Print task information. */
        printf("%s\t(%d)\t%lu\t%lu\t%lu\t%lu\r\n",
               tcb->name,
               tcb->class,
               tcb->stack_size,
               stack_used,
               tcb->stack_size - stack_used,
               (uint32_t)tcb->scheduled);

        /* Get the next task. */
        tcb = tcb->next_global;
    }

} /* util_print_sys_info */

#endif /* CONFIG_TASK_STATS */
