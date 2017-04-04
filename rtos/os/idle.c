/*
 * idle.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <idle.h>

/* Idle work definition. */
static IDLE_WORK idle_work[IDLE_WORK_MAX];

/* Definitions for idle task. */
static void idle_task_entry(void *);
static TASK idle_task;
static uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE];

/*
 * idle_task_init
 * This will initialize the idle task.
 */
void idle_task_init()
{
    /* Initialize idle task's control block and stack. */
    task_create(&idle_task, "Idle", idle_task_stack, IDLE_TASK_STACK_SIZE, &idle_task_entry, (void *)0x00, TASK_NO_RETURN);
    scheduler_task_add(&idle_task, 255);

} /* idle_task_init */

/*
 * idle_task_get
 * This will return the control block for the idle task.
 */
TASK *idle_task_get()
{
    /* Return control block for the idle task. */
    return (&idle_task);

} /* idle_task_get */

/*
 * idle_add_work
 * @do_fun: Function that is needed to be called in idle time.
 * @priv_data: Private data to be passed to the work function.
 * @return: Success will be returned if work was successfully added,
 *  IDLE_NO_SPACE will be returned if we don't have a free slot for this work.
 * This function will add work in the idle task. This work will be called in
 * the context on idle task with the provided data.
 */
int32_t idle_add_work(IDLE_DO *do_fun, void *priv_data)
{
    int32_t status = IDLE_NO_SPACE;
    uint32_t i;

    /* Lock the scheduler to protect work data. */
    scheduler_lock();

    /* Traverse the list of idle works. */
    for (i = 0; i < IDLE_WORK_MAX; i++)
    {
        /* If this is a free entry. */
        if (idle_work[i].do_fun == NULL)
        {
            /* Initialize the idle work. */
            idle_work[i].do_fun = do_fun;
            idle_work[i].priv_data = priv_data;

            /* Return success to the caller. */
            status = SUCCESS;

            /* Break out of this loop. */
            break;
        }
    }

    /* Unlock the scheduler. */
    scheduler_unlock();

    /* Return status to the caller. */
    return (status);

} /* idle_add_work */

/*
 * idle_remove_work
 * @do_fun: Function associated with work needed to be removed.
 * @priv_data: Private data associated with the work.
 * @return: Success will be returned if work was successfully removed,
 *  IDLE_NOT_FOUND will be returned if we did not find the required work.
 * This function will remove a previously added idle work.
 */
int32_t idle_remove_work(IDLE_DO *do_fun, void *priv_data)
{
    int32_t status = IDLE_NOT_FOUND;
    uint32_t i;

    /* Lock the scheduler to protect work data. */
    scheduler_lock();

    /* Traverse the list of idle works. */
    for (i = 0; i < IDLE_WORK_MAX; i++)
    {
        /* If this is the required entry. */
        if ((idle_work[i].do_fun == do_fun) && (idle_work[i].priv_data == priv_data))
        {
            /* Mark this work as invalid. */
            idle_work[i].do_fun = NULL;

            /* Return success to the caller. */
            status = SUCCESS;

            /* Break out of this loop. */
            break;
        }
    }

    /* Unlock the scheduler. */
    scheduler_unlock();

    /* Return status to the caller. */
    return (status);

} /* idle_remove_work */

/*
 * idle_task_entry
 * @argv: Unused parameter.
 * This is idle task that will run when there is no task in the system to run
 * at a particular instance.
 */
static void idle_task_entry(void *argv)
{
    IDLE_DO *do_fun;
    void *priv_data;
    uint32_t i;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Indefinitely perform idle work. */
    for (;;)
    {
        /* Traverse the idle work list. */
        for (i = 0; i < IDLE_WORK_MAX; i++)
        {
            /* Lock the scheduler to protect work data. */
            scheduler_lock();

            /* Save the work data. */
            do_fun = idle_work[i].do_fun;
            priv_data = idle_work[i].priv_data;

            /* Unlock the scheduler. */
            scheduler_unlock();

            /* If we do have a valid work. */
            if (do_fun != NULL)
            {
                /* Do required work. */
                do_fun(priv_data);
            }
        }
    }

} /* idle_task_entry */
