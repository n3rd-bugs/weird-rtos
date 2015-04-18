/*
 * net_condition.c
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

#ifdef CONFIG_NET
#include <semaphore.h>
#include <string.h>
#include <net_condition.h>
#include <net_buffer.h>

/* Global networking condition data. */
static NET_CONDITION net_condition_data;
static SUSPEND net_buffer_suspend;

/* Internal function prototypes. */
static int32_t net_condition_get_index(NET_CONDITION *, CONDITION *);
static void net_condition_task_entry(void *);

/* Networking condition task data. */
static TASK net_condition_tcb;
static char net_condition_stack[NET_COND_STACK_SIZE];

/*
 * net_condition_init
 * This will initialize networking stack conditions.
 */
void net_condition_init()
{
    CONDITION *condition;
    NET_CONDITION_PROCESS *process;

    /* Clear the networking stack condition global data. */
    memset(&net_condition_data, 0, sizeof(NET_CONDITION));

    /* Get networking buffer condition. */
    net_buffer_get_condition(&condition, &net_buffer_suspend, &process);

    /* Add networking buffer condition. */
    net_condition_add(condition, &net_buffer_suspend, process);

    /* Create a task to process the incoming networking buffers. */
    task_create(&net_condition_tcb, "NET-RX", net_condition_stack, NET_COND_STACK_SIZE, &net_condition_task_entry, (void *)(&net_condition_data), TASK_NO_RETURN);
    scheduler_task_add(&net_condition_tcb, TASK_APERIODIC, 5, 0);

} /* net_condition_init */

/*
 * net_condition_add
 * @condition: Already populated condition.
 * @suspend: Already populated suspend.
 * @process: Callback for this condition.
 * This will add a new condition that will be process by networking stack.
 */
void net_condition_add(CONDITION *condition, SUSPEND *suspend, NET_CONDITION_PROCESS *process)
{
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Should never happen. */
    OS_ASSERT(net_condition_data.num == NET_COND_NUM_TOTAL);

    /* Add this condition to the condition list. */
    net_condition_data.condition[net_condition_data.num] = condition;
    net_condition_data.suspend[net_condition_data.num] = suspend;
    net_condition_data.process[net_condition_data.num] = process;

    /* Increase the number of conditions. */
    net_condition_data.num++;

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

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
    int32_t index;
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();

    /* Validate that we do have a condition on which we are listening. */
    OS_ASSERT(net_condition_data.num <= 0);

    /* Get the index of the condition we need to remove. */
    index = net_condition_get_index(&net_condition_data, condition);

    /* Should never happen. */
    OS_ASSERT(index < 0);
    OS_ASSERT(index >= (int32_t)net_condition_data.num);

    /* Check if we need to move the data around to keep all conditions inline. */
    if (index != (int32_t)(net_condition_data.num - 1))
    {
        /* Align conditions. */
        memcpy(net_condition_data.condition[index], net_condition_data.condition[index + 1], (sizeof(CONDITION *) * ((net_condition_data.num - 1) - (uint32_t)index)));
        memcpy(net_condition_data.suspend[index], net_condition_data.suspend[index + 1], (sizeof(SUSPEND *) * ((net_condition_data.num - 1) - (uint32_t)index)));
        memcpy(net_condition_data.process[index], net_condition_data.process[index + 1], (sizeof(NET_CONDITION_PROCESS *) * ((net_condition_data.num - 1) - (uint32_t)index)));
    }

    /* Clear the last condition. */
    net_condition_data.condition[net_condition_data.num - 1] = NULL;
    net_condition_data.suspend[net_condition_data.num - 1] = NULL;
    net_condition_data.process[net_condition_data.num - 1] = NULL;

    /* Decrement the number of conditions. */
    net_condition_data.num = (uint32_t)(net_condition_data.num - 1);

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);

} /* net_condition_remove */

/*
 * net_condition_task_entry
 * @argv: Networking condition data needed to be processed.
 * This is task entry function to process networking conditions.
 */
static void net_condition_task_entry(void *argv)
{
    NET_CONDITION *net_cond = (NET_CONDITION *)argv;
    NET_CONDITION_PROCESS *process;
    uint32_t interrupt_level, num_condition;
    int32_t status;

    /* Should never happen. */
    OS_ASSERT(net_cond == NULL);

    /* This function should never return. */
    for (;;)
    {
        /* Disable global interrupts. */
        /* This is required to protect the condition list. */
        interrupt_level = GET_INTERRUPT_LEVEL();
        DISABLE_INTERRUPTS();

        /* Suspend until we have a condition to process. */
        num_condition = net_cond->num;
        status = suspend_condition(net_cond->condition[0], net_cond->suspend[0], MAX_WAIT, &num_condition, FALSE);

        /* If a condition was successful. */
        if ((status == SUCCESS) && (num_condition < net_cond->num))
        {
            /* Pick the condition data. */
            process = net_cond->process[num_condition];
        }

        /* We have no need to protect the networking conditions. */

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        /* Process this condition. */
        process();
    }

} /* net_condition_task_entry */

#endif /* CONFIG_NET */
