/*
 * net_condition.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_NET
#include <semaphore.h>
#include <string.h>
#include <net_condition.h>
#include <net_buffer.h>
#include <net_work.h>

/* Networking stack work queue. */
WORK_QUEUE net_work_queue;

/* Global networking condition data. */
static NET_CONDITION net_condition_data;
static SUSPEND net_buffer_suspend;

/* Internal function prototypes. */
static int32_t net_condition_get_index(NET_CONDITION *, CONDITION *);
static int32_t net_condition_do_remove(void *);
static void net_condition_task_entry(void *);

/* Networking condition task data. */
TASK net_condition_tcb;
static uint8_t net_condition_stack[NET_COND_STACK_SIZE];

/*
 * net_condition_init
 * This will initialize networking stack conditions.
 */
void net_condition_init(void)
{
    CONDITION *condition;
    NET_CONDITION_PROCESS *process;

    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* Clear the networking stack condition global data. */
    memset(&net_condition_data, 0, sizeof(NET_CONDITION));

    /* Get networking buffer condition. */
    net_buffer_get_condition(&condition, &net_buffer_suspend, &process);

    /* Add networking buffer condition. */
    net_condition_add(condition, &net_buffer_suspend, process, NULL);

    /* Create a task to process the incoming networking conditions. */
    task_create(&net_condition_tcb, P_STR("NET_COND"), net_condition_stack, NET_COND_STACK_SIZE, &net_condition_task_entry, (void *)(&net_condition_data), TASK_NO_RETURN);
    scheduler_task_add(&net_condition_tcb, 5);

    /* Initialize networking stack work queue. */
    net_work_init(&net_work_queue);

    SYS_LOG_FUNCTION_EXIT(NET_CONDITION);

} /* net_condition_init */

/*
 * net_condition_updated
 * This will refresh the networking conditions if required.
 */
void net_condition_updated(void)
{
    TASK *tcb = get_current_task();

    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* If this is not the networking task. */
    if ((tcb) && (tcb != &net_condition_tcb))
    {
        /* Resume the networking condition task to add this new condition. */

        /* Get lock for buffer file descriptor. */
        ASSERT(fd_get_lock(net_buff_fd) != SUCCESS);

        /* Set flag that new data is available on buffer file descriptor. */
        fd_data_available(net_buff_fd);

        /* Release lock for buffer file descriptor. */
        fd_release_lock(net_buff_fd);
    }

    SYS_LOG_FUNCTION_EXIT(NET_CONDITION);

} /* net_condition_updated */

/*
 * net_condition_add
 * @condition: Already populated condition.
 * @suspend: Already populated suspend.
 * @process: Callback for this condition.
 * @data: Data needed to be forwarded to the callback.
 * This will add a new condition that will be process by networking stack.
 */
void net_condition_add(CONDITION *condition, SUSPEND *suspend, NET_CONDITION_PROCESS *process, void *data)
{
    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* Should never happen. */
    ASSERT(net_condition_data.num == NET_COND_NUM_TOTAL);

    /* Add this condition to the condition list. */
    net_condition_data.condition[net_condition_data.num] = condition;
    net_condition_data.suspend[net_condition_data.num] = suspend;
    net_condition_data.process[net_condition_data.num] = process;
    net_condition_data.data[net_condition_data.num] = data;

    SYS_LOG_FUNCTION_MSG(NET_CONDITION, SYS_LOG_DEBUG, "added a new condition at %d", net_condition_data.num);

    /* Increase the number of conditions. */
    net_condition_data.num++;

    /* Networking condition data has been updated. */
    net_condition_updated();

    SYS_LOG_FUNCTION_EXIT(NET_CONDITION);

} /* net_condition_add */

/*
 * net_condition_get_index
 * @net_cond: Networking condition data.
 * @condition: Condition needed to be searched.
 * This will return the index of a networking condition in the networking
 * condition list.
 */
static int32_t net_condition_get_index(NET_CONDITION *net_cond, CONDITION *condtion)
{
    int32_t n, ret_index = -1;

    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* Go through all the valid conditions. */
    for (n = 0; n < (int32_t)net_cond->num; n++)
    {
        /* Check if this is the required condition. */
        if (condtion == net_cond->condition[n])
        {
            /* Return index of this condition. */
            ret_index = n;

            /* Break out of this loop. */
            break;
        }
    }

    SYS_LOG_FUNCTION_EXIT(NET_CONDITION);

    /* Return the required condition. */
    return (ret_index);

} /* net_condition_get_index */

/*
 * net_condition_remove
 * @condition: Condition needed to be removed.
 * This will remove an existing condition from the networking stack.
 */
void net_condition_remove(CONDITION *condition)
{
    TASK *tcb = get_current_task();

    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* If this is not networking stack. */
    if (tcb != &net_condition_tcb)
    {
        /* Lets remove this condition in the context of networking stack. */
        net_work_add(&net_work_queue, NULL, &net_condition_do_remove, condition, MAX_WAIT);
    }
    else
    {
        /* Lets remove this condition. */
        net_condition_do_remove(condition);
    }

    SYS_LOG_FUNCTION_EXIT(NET_CONDITION);

} /* net_condition_remove */

/*
 * net_condition_do_remove
 * @condition: Condition needed to be removed.
 * This is work call back that will remove an existing condition from the
 * networking stack.
 */
static int32_t net_condition_do_remove(void *data)
{
    CONDITION *condition = (CONDITION *)data;
    int32_t index;
    TASK *tcb = get_current_task();

    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* If this is not the networking task. */
    ASSERT(((!tcb) || (tcb != &net_condition_tcb)));

    /* Validate that we do have a condition on which we are listening. */
    ASSERT(net_condition_data.num <= 0);

    /* Get the index of the condition we need to remove. */
    index = net_condition_get_index(&net_condition_data, condition);

    /* Should never happen. */
    ASSERT(index < 0);
    ASSERT(index >= (int32_t)net_condition_data.num);

    /* Check if we need to move the data around to keep all conditions inline. */
    if (index != (int32_t)(net_condition_data.num - 1))
    {
        /* Align conditions. */
        memcpy(&net_condition_data.condition[index], &net_condition_data.condition[index + 1], (sizeof(CONDITION *) * (uint8_t)((net_condition_data.num - 1) - index)));
        memcpy(&net_condition_data.suspend[index], &net_condition_data.suspend[index + 1], (sizeof(SUSPEND *) * (uint8_t)((net_condition_data.num - 1) - (uint8_t)index)));
        memcpy(&net_condition_data.process[index], &net_condition_data.process[index + 1], (sizeof(NET_CONDITION_PROCESS *) * (uint8_t)((net_condition_data.num - 1) - index)));
        memcpy(&net_condition_data.data[index], &net_condition_data.data[index + 1], (sizeof(NET_CONDITION_PROCESS *) * (uint8_t)((net_condition_data.num - 1) - index)));
    }

    /* Clear the last condition. */
    net_condition_data.condition[net_condition_data.num - 1] = NULL;
    net_condition_data.suspend[net_condition_data.num - 1] = NULL;
    net_condition_data.process[net_condition_data.num - 1] = NULL;
    net_condition_data.data[net_condition_data.num - 1] = NULL;

    /* Decrement the number of conditions. */
    net_condition_data.num = (uint8_t)(net_condition_data.num - 1);

    SYS_LOG_FUNCTION_MSG(NET_CONDITION, SYS_LOG_DEBUG, "removed a condition %d", index);

    SYS_LOG_FUNCTION_EXIT(NET_CONDITION);

    /* Always return success. */
    return (SUCCESS);

} /* net_condition_do_remove */

/*
 * net_condition_task_entry
 * @argv: Networking condition data needed to be processed.
 * This is task entry function to process networking conditions.
 */
static void net_condition_task_entry(void *argv)
{
    NET_CONDITION *net_cond = (NET_CONDITION *)argv;
    int32_t status;
    uint8_t num_condition;

    SYS_LOG_FUNCTION_ENTRY(NET_CONDITION);

    /* Should never happen. */
    ASSERT(net_cond == NULL);

    /* This function should never return. */
    for (;;)
    {
        SYS_LOG_FUNCTION_MSG(NET_CONDITION, SYS_LOG_DEBUG, "suspending on conditions %d", net_cond->num);

        /* Suspend until we have a condition to process. */
        num_condition = net_cond->num;
        status = suspend_condition(net_cond->condition, net_cond->suspend, &num_condition, FALSE);

        SYS_LOG_FUNCTION_MSG(NET_CONDITION, SYS_LOG_DEBUG, "got a condition to process %d", num_condition);

        /* If a condition was successful became valid. */
        if (num_condition < net_cond->num)
        {
            /* Process this condition. */
            net_cond->process[num_condition](net_cond->data[num_condition], status);
        }

        SYS_LOG_FUNCTION_MSG(NET_CONDITION, SYS_LOG_DEBUG, "processed the condition %d", num_condition);
    }

} /* net_condition_task_entry */

#endif /* CONFIG_NET */
