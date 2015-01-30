/*
 * fs.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#include <os.h>
#include <string.h>
#include <sll.h>
#include <path.h>
#include <sleep.h>

#ifdef CONFIG_FS

/* Global variables. */
static FS_DATA file_data;

/*
 * fs_init
 * This function will initialize file system layer. This function must be
 * called before using any other APIs.
 */
void fs_init()
{
    /* Clear the global file system data. */
    memset(&file_data, 0, sizeof(FS_DATA));

#ifdef CONFIG_SEMAPHORE
    /* Create a semaphore to protect global file system data. */
    semaphore_create(&file_data.lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

#ifdef FS_CONSOLE
    /* Initialize console. */
    console_init();
#endif

#ifdef FS_PIPE
    /* Initialize PIPE file system. */
    pipe_init();
#endif

} /* fs_init */

/*
 * fs_register
 * @file_system: File system data to be registered.
 * This function registers a file system, this is called by lower layer to
 * register a new file system.
 */
void fs_register(FS *file_system)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&file_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* Just push this file system in the list. */
    sll_push(&file_data.list, file_system, OFFSETOF(FS, next));

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&file_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* fs_register */

/*
 * fs_unregister
 * @file_system: File system data needed to unregistered.
 * This function will unregister a file system. Must be called by the
 * component that has registered the file system.
 * Note that the invalidating a file descriptor is needed to be maintained by
 * underlying layer.
 */
void fs_unregister(FS *file_system)
{
#ifndef CONFIG_SEMAPHORE
    /* Lock the scheduler. */
    scheduler_lock();
#else
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&file_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* Just remove this file system from the list. */
    sll_remove(&file_data.list, file_system, OFFSETOF(FS, next));

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&file_data.lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif

} /* fs_unregister */

/*
 * fs_sreach_directory
 * @node: An existing file system in the list.
 * @param: Search parameter that will be updated.
 * @return: FALSE.
 * This is a search function to search a file system that should be used
 * to process a given node.
 */
uint8_t fs_sreach_directory(void *node, void *param)
{
    /* Save the required path. */
    char *path = ((DIR_PARAM *)param)->name;
    uint32_t match = FALSE;

    /* Match the file system path. */
    if (util_path_match(((FS *)node)->name, &path) == TRUE)
    {
        /* If given path was is a directory. */
        if (*path == '\\')
        {
            /* Move past the delimiter. */
            path++;
        }

        /* If this was an exact match. */
        if (*path == '\0')
        {
            /* Got an exact match. */
            match = TRUE;
        }

        /* Update the pointer until this path was matched. */
        ((DIR_PARAM *)param)->matched = path;

        /* Save this node. */
        ((DIR_PARAM *)param)->priv = node;
    }

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* fs_sreach_directory */

/*
 * fs_sreach_node
 * @node: An existing file system in the list.
 * @param: Search parameter that will be updated.
 * @return: FALSE.
 * This is a search function to search a file system that should be used
 * to process a given node.
 */
uint8_t fs_sreach_node(void *node, void *param)
{
    /* Save the required path. */
    char *path = ((NODE_PARAM *)param)->name;
    uint32_t match = FALSE;

    /* Match the file system path and this is a exact match. */
    if ( (util_path_match(((FS *)node)->name, &path) == TRUE) && (*path == '\0') )
    {
        /* Return this node. */
        ((NODE_PARAM *)param)->priv = node;

        /* Got an match. */
        match = TRUE;
    }

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* fs_sreach_node */

/*
 * fs_open
 * @name: File name to open.
 * @flags: Open flags.
 * This function opens a named node with given flags. The name should not
 * end with a '\\'.
 */
FD fs_open(char *name, uint32_t flags)
{
    DIR_PARAM param;
    FD fd = 0;

#ifdef CONFIG_SEMAPHORE
    /* Obtain the global data lock. */
    OS_ASSERT(semaphore_obtain(&file_data.lock, MAX_WAIT) != SUCCESS);
#endif

    /* Initialize a search parameter. */
    param.name = name;
    param.matched = NULL;
    param.priv = (void *)fd;

    /* First find a file system to which this call can be forwarded. */
    sll_search(&file_data.list, NULL, fs_sreach_directory, &param, OFFSETOF(FS, next));

    /* If a node was found. */
    if (param.priv)
    {
        /* Update the name to the resolved. */
        name = param.matched;

        /* Use this FD, we will update it if required. */
        fd = (FD)param.priv;
    }

#ifdef CONFIG_SEMAPHORE
    /* Release the global data lock. */
    semaphore_release(&file_data.lock);
#endif

    if (fd != 0)
    {
        /* Check if we need to call the underlying function to get a new file
         * descriptor. */
        if (((FS *)fd)->open != NULL)
        {
            /* Call the underlying API to get the file descriptor. */
            fd = (FD)((FS *)fd)->open(name, flags);
        }
    }

    /* Return the created file descriptor. */
    return (fd);

} /* fs_open */

/*
 * fs_close
 * @fd: Pointer to file descriptor.
 * This function will close a file descriptor.
 */
void fs_close(FD *fd)
{
    /* Check if a close function was registered with this descriptor. */
    if (((FS *)fd)->close != NULL)
    {
        /* Transfer call to underlying API. */
        ((FS *)fd)->close((void **)fd);
    }

    else
    {
        /* Clear the file descriptor. */
        *fd = (FD)NULL;
    }

} /* fs_close */

/*
 * fs_read
 * @fd: File descriptor.
 * @buffer: Data buffer.
 * @nbytes: Number of bytes that can be read in the provided buffer.
 * @return: >0: number of bytes actually read in the given buffer,
 *  FS_NODE_DELETED if file node was deleted during waiting for read.
 * This function will read data from a file descriptor.
 */
int32_t fs_read(FD fd, char *buffer, uint32_t nbytes)
{
    int32_t read = SUCCESS;
#ifdef CONFIG_SLEEP
    uint32_t last_tick = current_system_tick();
#endif
    TASK *tcb;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        read = ((FS *)fd)->get_lock((void *)fd);
    }

    /* If lock was successfully obtained. */
    if (read == SUCCESS)
    {
        /* Check if a read function was registered with this descriptor */
        if ((((FS *)fd)->read != NULL))
        {
            /* Check if we need to block on read for this FS and there is no data on
             * the socket, and this is being called from a task. */
            if ((!(((FS *)fd)->flags & FS_DATA_AVAILABLE)) &&
                (((FS *)fd)->flags & FS_BLOCK) &&
                (current_task))
            {
                /* Get executing task. */
                tcb = get_current_task();

    #ifdef CONFIG_SLEEP
                /* Check if we need to wait for a finite time. */
                if (((FS *)fd)->timeout != (uint32_t)(MAX_WAIT))
                {
                    /* Add the current task to the sleep list, if data is not
                     * available in the allowed time the task will be resumed. */
                    sleep_add_to_list(tcb, ((FS *)fd)->timeout);
                }
    #endif /* CONFIG_SLEEP */

                /* There is never a surety that if some data is available and
                 * picked up by a waiting task as scheduler might decide to run
                 * some other higher/same priority task and data can get consumed
                 * by it before this waiting task can get it. */
                /* This is not a bug and happen in a RTOS where different types of
                 * schedulers are present. */
                do
                {
    #ifdef CONFIG_SLEEP
                    /* Check if we need to wait for a finite time. */
                    if (((FS *)fd)->timeout != (uint32_t)(MAX_WAIT))
                    {
                        /* Add the current task to the sleep list, if not available in
                         * the allowed time the task will be resumed. */
                        sleep_add_to_list(tcb, ((FS *)fd)->timeout - (current_system_tick() - last_tick));

                        /* Save when we suspended last time. */
                        last_tick = current_system_tick();
                    }
    #endif /* CONFIG_SLEEP */

                    /* If we need to sort the list on priority. */
                    if (((FS *)fd)->flags & FS_PRIORITY_SORT)
                    {
                        /* Add this task on the file descriptor task list. */
                        sll_insert(&((FS *)fd)->task_list, tcb, &task_priority_sort, OFFSETOF(TASK, next));
                    }

                    else
                    {
                        /* Add this task at the end of task list. */
                        sll_append(&((FS *)fd)->task_list, tcb, OFFSETOF(TASK, next));
                    }

                    /* We need to suspend so disable preemption and release the lock
                     * this way releasing the lock will not pass the control to
                     * next task. */

                    /* Disable preemption. */
                    scheduler_lock();

                    if (((FS *)fd)->release_lock)
                    {
                        /* Release lock for this file descriptor. */
                        ((FS *)fd)->release_lock((void *)fd);
                    }

                    /* Task is being suspended. */
                    tcb->status = TASK_SUSPENDED;

                    /* Wait for either being resumed by some data or timeout. */
                    task_waiting();

                    /* Check if we are resumed due to a timeout. */
                    if (tcb->status == TASK_RESUME_SLEEP)
                    {
                        /* Remove this task from the file descriptor task list. */
                        sll_remove(&((FS *)fd)->task_list, tcb, OFFSETOF(TASK, next));
                    }

                    /* Enable preemption. */
                    scheduler_unlock();

                    if (tcb->status == TASK_RESUME_ERROR)
                    {
                        /* The file node is being deleted. */
                        read = FS_NODE_DELETED;
                        break;
                    }

                    if (((FS *)fd)->get_lock)
                    {
                        /* Get lock for this file descriptor. */
                        read = ((FS *)fd)->get_lock((void *)fd);

                        /* If we were not able to get the lock. */
                        if (read != SUCCESS)
                        {
                            break;
                        }
                    }

                } while (!(((FS *)fd)->flags & FS_DATA_AVAILABLE));
            }

            /* Check if some data is available. */
            if ((read == SUCCESS) && (((FS *)fd)->flags & FS_DATA_AVAILABLE))
            {
                /* Transfer call to underlying API. */
                read = ((FS *)fd)->read((void *)fd, buffer, nbytes);
            }
        }

        if ((read != SEMAPHORE_DELETED) && (read != FS_NODE_DELETED) &&
            (((FS *)fd)->release_lock))
        {
            /* Release lock for this file descriptor. */
            ((FS *)fd)->release_lock((void *)fd);
        }
    }

    /* Return number of bytes read. */
    return (read);

} /* fs_read */

/*
 * fs_write
 * @fd: File descriptor.
 * @buffer: Data buffer.
 * @nbytes: Number of bytes to write.
 * This function will write data on a file descriptor.
 */
int32_t fs_write(FD fd, char *buffer, uint32_t nbytes)
{
    int32_t written = SUCCESS;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        written = ((FS *)fd)->get_lock((void *)fd);
    }

    /* If lock was successfully obtained. */
    if (written == SUCCESS)
    {
        /* Check if a write function was registered with this descriptor. */
        if (((FS *)fd)->write != NULL)
        {
            /* Transfer call to underlying API. */
            written = ((FS *)fd)->write((void *)fd, buffer, nbytes);
        }

        if (((FS *)fd)->release_lock)
        {
            /* Release lock for this file descriptor. */
            ((FS *)fd)->release_lock((void *)fd);
        }
    }

    /* Return number of bytes written. */
    return (written);

} /* fs_write */

/*
 * fs_ioctl
 * @fd: File descriptor.
 * @cmd: IOCTL command needed to be executed.
 * @param: IOCTL command parameter if any.
 * This function will execute a command on a file descriptor.
 */
int32_t fs_ioctl(FD fd, uint32_t cmd, void *param)
{
    int32_t ret = SUCCESS;

    if (((FS *)fd)->get_lock)
    {
        /* Get lock for this file descriptor. */
        ret = ((FS *)fd)->get_lock((void *)fd);
    }

    /* If lock was successfully obtained. */
    if (ret == SUCCESS)
    {
        /* Check if an IOCTL function was registered with this descriptor. */
        if (((FS *)fd)->ioctl != NULL)
        {
            /* Transfer call to underlying API. */
            ret = ((FS *)fd)->ioctl((void *)fd, cmd, param);
        }

        if (((FS *)fd)->release_lock)
        {
            /* Release lock for this file descriptor. */
            ((FS *)fd)->release_lock((void *)fd);
        }
    }

    return (ret);

} /* fs_ioctl */

/*
 * fd_sreach_task
 * @node: An task waiting on this file system.
 * @param: Resumption criteria.
 * @return: TRUE if we need to resume this task, FALSE if we cannot resume
 *  this task.
 * This is a search function to search a task that satisfy the suspension
 * criteria set by underlying file system.
 */
static uint8_t fd_sreach_task(void *node, void *param)
{
    FS_PARAM *fs_param = (FS_PARAM *)param;
    uint32_t match = FALSE;

    /* Call the FS function to see if we can resume this task. */
    if (((FS *)fs_param->fs)->should_resume(fs_param->fs, fs_param->param, ((TASK *)node)->suspend_data))
    {
        /* Got an match. */
        match = TRUE;
    }

    /* Return if we need to stop the search or need to process more. */
    return (match);

} /* fs_sreach_task */

/*
 * fd_data_available
 * @fd: File descriptor for which new data is available.
 * @param: If required a parameter can be passed to check if a task is eligible
 *  for resumption.
 * This function will set the data available flag for a file descriptor
 * and resume any tasks waiting for it. Caller must have the lock for FS before
 * calling this routine.
 */
void fd_data_available(void *fd, FS_PARAM *param)
{
    TASK *tcb = NULL;

    /* Set flag that some data is available. */
    ((FS *)fd)->flags |= FS_DATA_AVAILABLE;

    /* If a parameter was given. */
    if (param != NULL)
    {
        /* Push this FS on the parameter. */
        param->fs = fd;

        /* Search for a task that can be resumed. */
        tcb = (TASK *)sll_search_pop(&((FS *)fd)->task_list, fd_sreach_task, param, OFFSETOF(TASK, next));
    }

    else
    {
        /* Get the first task that can be executed. */
        tcb = (TASK *)sll_pop(&((FS *)fd)->task_list, OFFSETOF(TASK, next));
    }

    /* If we have a task that can be resumed. */
    if (tcb)
    {
        /* Task is resuming as some data is now available. */
        tcb->status = TASK_RESUME;

#ifdef CONFIG_SLEEP
        /* Remove this task from sleeping tasks. */
        sleep_remove_from_list(tcb);
#endif /* CONFIG_SLEEP */

        /* Try to reschedule this task. */
        ((SCHEDULER *)(tcb->scheduler))->yield(tcb, YIELD_SYSTEM);

        /* This function might have been called from an IRQ. */
        if (current_task)
        {
            /* Yield the current task and schedule the new task if required. */
            task_yield();
        }
    }

} /* fd_data_available */

/*
 * fd_data_flushed
 * @fs: File descriptor for which data has been flushed.
 * This function will clear the data available flag for a given file
 * descriptor. Caller must have the lock for FS before calling this routine.
 */
void fd_data_flushed(void *fd)
{
    /* Clear the data available flag. */
    ((FS *)fd)->flags &= ~(FS_DATA_AVAILABLE);

} /* fd_data_flushed */

/*
 * fs_resume_all
 * @fs: File system for which all waiting tasks are needed to be resumed.
 * This function will resume all the tasks waiting on an file descriptor with
 * an error.
 */
void fs_resume_all(void *fd)
{
    /* Get the first task that can be resumed. */
    TASK *tcb = (TASK *)sll_pop(&((FS *)fd)->task_list, OFFSETOF(TASK, next));

    /* Resume all tasks in this list. */
    while (tcb)
    {
        /* Task is resuming because of an error. */
        tcb->status = TASK_RESUME_ERROR;

#ifdef CONFIG_SLEEP
        /* Remove this task from sleeping tasks. */
        sleep_remove_from_list(tcb);
#endif /* CONFIG_SLEEP */

        /* Try to reschedule this task. */
        ((SCHEDULER *)(tcb->scheduler))->yield(tcb, YIELD_SYSTEM);

        /* This function might have been called from an IRQ. */
        if (current_task)
        {
            /* Yield the current task and schedule the new task if required. */
            task_yield();
        }

        /* Get the next task that can be resumed. */
        tcb = (TASK *)sll_pop(&((FS *)fd)->task_list, OFFSETOF(TASK, next));
    }

} /* fs_resume_all */

#endif /* CONFIG_FS */
