/*
 * tasks.c
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
#include <string.h>
#include <kernel.h>
#include <tasks.h>

/* Internal function prototypes. */
static void task_entry_return(void *);

/*
 * task_create
 * @tcb: Task control block for the new task.
 * @name: Name for this task.
 * @stack: Task stack that will be used to run this task.
 * @stack_size: Task stack size, that should be used for this task.
 * @entry: Task entry function.
 * @argv: Any arguments that will be passed to the task.
 * @flags: Flags for this task.
 * This function initializes a task control block with the given parameters, that
 * can be then enqueued in the scheduler to run.
 */
void task_create(TASK *tcb, P_STR_T name, uint8_t *stack, uint32_t stack_size, TASK_ENTRY *entry, void *argv, uint8_t flags)
{
    /* Clear the task structure. */
    memset(tcb, 0, sizeof(TASK));

#ifdef CONFIG_TASK_STATS
    /* If user has provided a task name. */
    tcb->name = name;

    /* Fill the task stack with a pre-defined constant. */
    memset(stack, CONFIG_STACK_PATTERN, stack_size);

    /* Store the stack information for this task. */
    tcb->stack_size = stack_size;
    tcb->stack_start = stack;
#else
    /* Remove some warnings. */
    UNUSED_PARAM(name);
    UNUSED_PARAM(stack);
    UNUSED_PARAM(stack_size);
#endif /* CONFIG_TASK_STATS */

    /* Initialize task information. */
    tcb->entry = entry;
    tcb->argv = argv;
    tcb->flags = flags;
    tcb->status = TASK_FINISHED;

    /* Adjust task's stack pointer. */
    TOS_SET(tcb->tos, stack, stack_size);

    if (tcb->flags & TASK_NO_RETURN)
    {
        /* Initialize task's stack. */
        stack_init(tcb, entry, argv);
    }
    else
    {
        /* Initialize task's stack. */
        stack_init(tcb, &task_entry_return, argv);
    }

} /* task_create */

/*
 * task_entry_return
 * @node: Argument needed to be passed to the task.
 * This is entry and return function for the tasks which can finish.
 */
static void task_entry_return(void *argv)
{
    TASK *tcb = get_current_task();

    /* We will run the task until we are actually killed. */
    while (TRUE)
    {
        /* When entering run this task. */
        get_current_task()->entry(argv);

        /* The task will be resumed by the application using scheduler_task_add,
         * that may also change the class of this task. */

        /* Lock the interrupts we will enable them again when transferring
         * control to system. */
        DISABLE_INTERRUPTS();

        /* Update the task status that it is now finished. */
        tcb->status = TASK_FINISHED;

        /* Task is now the property of the initializer. */
        CONTROL_TO_SYSTEM();
    }

} /* task_entry_return. */
