/*
 * tasks.c
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
#include <string.h>
#include <os.h>
#include <tasks.h>

/*
 * task_create
 * @tcb: Task control block for the new task.
 * @name: Name for this task.
 * @stack: Task stack that will be used to run this task.
 * @stack_size: Task stack size, that should be used for this task.
 * @entry: Task entry function.
 * @argv: Any arguments that will be passed to the task.
 * This function initializes a task control block with the given parameters, that
 * can be then enqueued in the scheduler to run.
 */
void task_create(TASK *tcb, char *name, char *stack, uint32_t stack_size, TASK_ENTRY *entry, void *argv)
{
    /* Clear the task structure. */
    memset(tcb, 0, sizeof(TASK));

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* If user has provided a task name. */
    if (name != 0)
    {
        /* Copy the task name. */
        strncpy(tcb->name, name, 7);
    }

    /* Store the stack information for this task. */
    tcb->stack_size = stack_size;
    tcb->stack_start = stack;

    /* Fill the task stack with a pre-defined constant. */
    memset(stack, CONFIG_STACK_FILL, stack_size);
#else
    /* Remove some warnings. */
    UNUSED_PARAM(name);
#endif /* CONFIG_INCLUDE_TASK_STATS */

    /* Adjust task's stack pointer. */
    TOS_SET(tcb->tos, stack, stack_size);

    /* Initialize task's stack. */
    os_stack_init(tcb, entry, argv);

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Break the task stack pattern. */
    *(tcb->tos) = 0x00;
#endif /* CONFIG_INCLUDE_TASK_STATS */

} /* task_create */
