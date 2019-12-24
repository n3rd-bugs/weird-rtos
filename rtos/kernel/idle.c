/*
 * idle.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <idle.h>
#include <semaphore.h>

#if (IDLE_WORK_MAX > 0)
/* Idle work definition. */
static IDLE_WORK idle_work[IDLE_WORK_MAX];
#ifdef IDLE_RUNTIME_UPDATE
static INTLCK idle_work_lock;
#endif /* IDLE_RUNTIME_UPDATE */
#endif /* (IDLE_WORK_MAX > 0) */

/* Definitions for idle task. */
TASK idle_task;
static void idle_task_entry(void *);
static uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE];
/*
 * idle_task_init
 * This will initialize the idle task.
 */
void idle_task_init(void)
{
    /* Initialize idle task's control block and stack. */
    task_create(&idle_task, P_STR("Idle"), idle_task_stack, IDLE_TASK_STACK_SIZE, &idle_task_entry, (void *)0x00, TASK_NO_RETURN);
    scheduler_task_add(&idle_task, (SCHEDULER_MAX_PRI + 1));

#if ((IDLE_WORK_MAX > 0) && defined(IDLE_RUNTIME_UPDATE))
    /* Initialize IDL work lock. */
    INTLCK_INIT(idle_work_lock);
#endif /* ((IDLE_WORK_MAX > 0) && defined(IDLE_RUNTIME_UPDATE)) */

} /* idle_task_init */

/*
 * idle_task_get
 * This will return the control block for the idle task.
 */
TASK *idle_task_get(void)
{
    /* Return control block for the idle task. */
    return (&idle_task);

} /* idle_task_get */

/*
 * idle_add_work
 * @do_fun: Function that is needed to be called in idle time.
 * @priv_data: Private data to be passed to the work function.
 * @return: Success will be returned if work was successfully added,
 *  IDLE_NO_SPACE will be returned if we don't have a free slot for this work,
 *  IDLE_CANNOT_UPDATE will be returned if kernel is in running state and
 *      runtime work updates are disabled.
 * This function will add work in the idle task. This work will be called in
 * the context on idle task with the provided data.
 */
int32_t idle_add_work(IDLE_DO *do_fun, void *priv_data)
{
    int32_t status = IDLE_NO_SPACE;
#if (IDLE_WORK_MAX > 0)
#ifdef IDLE_RUNTIME_UPDATE
    uint8_t acquired = FALSE;
#endif /* IDLE_RUNTIME_UPDATE */
    uint8_t i;

#ifndef IDLE_RUNTIME_UPDATE
    if (KERNEL_RUNNING() == FALSE)
#endif /* IDLE_RUNTIME_UPDATE */
    {
#ifdef IDLE_RUNTIME_UPDATE
        /* While we could not acquire the lock. */
        while (acquired == FALSE)
        {
            /* Try to acquire the lock. */
            INTLCK_TRY_GET(idle_work_lock, acquired);

            /* If lock was acquired. */
            if (acquired == FALSE)
            {
                /* Sleep and hope we get this lock in next try. */
                sleep_ticks(1);
            }
        }
#endif /* IDLE_RUNTIME_UPDATE */

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

#ifdef IDLE_RUNTIME_UPDATE
        /* Release the lock. */
        INTLCK_RELEASE(idle_work_lock);
#endif /* IDLE_RUNTIME_UPDATE */
    }
#ifndef IDLE_RUNTIME_UPDATE
    else
    {
        /* Kernel is in running state so we cannot remove any work. */
        status = IDLE_CANNOT_UPDATE;
    }
#endif /* IDLE_RUNTIME_UPDATE */
#else
    /* Remove compiler warnings. */
    UNUSED_PARAM(do_fun);
    UNUSED_PARAM(priv_data);
#endif /* (IDLE_WORK_MAX >  0) */

    /* Return status to the caller. */
    return (status);

} /* idle_add_work */

/*
 * idle_remove_work
 * @do_fun: Function associated with work needed to be removed.
 * @priv_data: Private data associated with the work.
 * @return: Success will be returned if work was successfully removed,
 *  IDLE_NOT_FOUND will be returned if we did not find the required work,
 *  IDLE_CANNOT_UPDATE will be returned if kernel is in running state and
 *      runtime work updates are disabled.
 * This function will remove a previously added idle work.
 */
int32_t idle_remove_work(IDLE_DO *do_fun, void *priv_data)
{
    int32_t status = IDLE_NOT_FOUND;
#if (IDLE_WORK_MAX >  0)
#ifdef IDLE_RUNTIME_UPDATE
    uint8_t acquired = FALSE;
#endif /* IDLE_RUNTIME_UPDATE */
    uint8_t i;

#ifndef IDLE_RUNTIME_UPDATE
    if (KERNEL_RUNNING() == FALSE)
#endif /* IDLE_RUNTIME_UPDATE */
    {
#ifdef IDLE_RUNTIME_UPDATE
        /* While we could not acquire the lock. */
        while (acquired == FALSE)
        {
            /* Try to acquire the lock. */
            INTLCK_TRY_GET(idle_work_lock, acquired);

            /* If lock was acquired. */
            if (acquired == FALSE)
            {
                /* Sleep and hope we get this lock in next try. */
                sleep_ticks(1);
            }
        }
#endif /* IDLE_RUNTIME_UPDATE */

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

#ifdef IDLE_RUNTIME_UPDATE
        /* Release the lock. */
        INTLCK_RELEASE(idle_work_lock);
#endif /* IDLE_RUNTIME_UPDATE */
    }
#ifndef IDLE_RUNTIME_UPDATE
    else
    {
        /* Kernel is in running state so we cannot add any more work. */
        status = IDLE_CANNOT_UPDATE;
    }
#endif /* IDLE_RUNTIME_UPDATE */
#else
    /* Remove compiler warnings. */
    UNUSED_PARAM(do_fun);
    UNUSED_PARAM(priv_data);
#endif /* (IDLE_WORK_MAX >  0) */

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
#if (IDLE_WORK_MAX >  0)
    IDLE_DO *do_fun;
    void *priv_data;
    uint8_t i;
#ifdef IDLE_RUNTIME_UPDATE
    uint8_t acquired;
#endif /* IDLE_RUNTIME_UPDATE */
#endif /* (IDLE_WORK_MAX >  0) */

    /* Remove some compiler warnings. */
    UNUSED_PARAM(argv);

    /* Indefinitely perform idle work. */
    for (;;)
    {
#if (IDLE_WORK_MAX >  0)
        /* Traverse the idle work list. */
        for (i = 0; i < IDLE_WORK_MAX; i++)
        {
#ifdef IDLE_RUNTIME_UPDATE
            /* While we could not acquire the lock. */
            acquired = FALSE;
            while (acquired == FALSE)
            {
                /* Try to acquire the lock. */
                INTLCK_TRY_GET(idle_work_lock, acquired);
            }
#endif /* IDLE_RUNTIME_UPDATE */

            /* Save the work data. */
            do_fun = idle_work[i].do_fun;
            priv_data = idle_work[i].priv_data;

#ifdef IDLE_RUNTIME_UPDATE
            /* Release the lock. */
            INTLCK_RELEASE(idle_work_lock);
#endif /* IDLE_RUNTIME_UPDATE */

            /* If we do have a valid work. */
            if (do_fun != NULL)
            {
                /* Do required work. */
                do_fun(priv_data);
            }
        }
#endif /* (IDLE_WORK_MAX >  0) */
    }

} /* idle_task_entry */
