#include <os.h>
#include <sys_info.h>
#include <stdio.h>

#ifdef CONFIG_INCLUDE_TASK_STATS

uint32_t util_task_calc_free_stack(TASK *tcb)
{
    uint32_t free = 0;

    /* Calculate the number of bytes intact on this task's
     * stack. */
    while (tcb->stack_start[free] == CONFIG_STACK_FILL)
    {
        free ++;
    }

    /* Return number of free bytes on the stack. */
    return (free);

} /* util_task_calc_free_stack */

void util_print_sys_info()
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_used;

    /* Print current system tick. */
    printf("System tick: %lu\n", (uint32_t)current_system_tick());

    /* Print table header. */
    printf("Name\tClass\tTotal\tFree\tMin.\tn(T)\n");

    /* Print information about all the tasks in the system. */
    while (tcb != 0)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_used = util_task_calc_free_stack(tcb);

        /* Print task information. */
        printf("%s\t(%d)\t%lu\t%lu\t%lu\t%lu\n",
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

#else

uint32_t util_task_calc_free_stack(TASK *tcb)
{
    return 0;
}

void util_print_sys_info()
{
    ;
}

#endif /* CONFIG_INCLUDE_TASK_STATS */
