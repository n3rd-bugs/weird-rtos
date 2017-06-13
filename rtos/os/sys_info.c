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
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#include <stdio.h>
#include <os.h>
#include <sys_info.h>

#ifdef CONFIG_TASK_STATS
#ifdef CONFIG_FS
#include <fs.h>
#include <string.h>
#endif

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

#ifdef SYS_STACK_SIZE
/*
 * util_system_calc_free_stack
 * This function returns the number of bytes used on the system stack.
 */
uint32_t util_system_calc_free_stack()
{
    uint32_t free = 0;

    /* Calculate the number of bytes intact on the system stack. */
    while (SYSTEM_STACK[free] == CONFIG_STACK_PATTERN)
    {
        free ++;
    }

    /* Return number of free bytes on the stack. */
    return (free);

} /* util_system_calc_free_stack */
#endif

/*
 * util_print_sys_info
 * This function prints generalized information about the operating system.
 */
void util_print_sys_info()
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_free;

    /* Print current system tick. */
    printf("System tick: %lu\r\n", (uint32_t)current_system_tick());

    /* Print table header. */
    printf("Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\r\n");

    /* Print information about all the tasks in the system. */
    while (tcb != NULL)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

        /* Print task information. */
        printf("%s\t%lu\t%lu\t%lu\t%li\t%lu\t%lu%s\r\n",
               tcb->name,
               tcb->stack_size,
               stack_free,
               tcb->stack_size - stack_free,
               tcb->status,
               (uint32_t)tcb->scheduled,
               (uint32_t)tcb->tick_sleep,
               (tcb == get_current_task()) ? "\t<Running>" : "");

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack();

    /* Print system stack information. */
    printf("SYSTEM\t%lu\t%lu\t%lu\t-\t-\t-\r\n",
           (uint32_t)SYS_STACK_SIZE,
           stack_free,
           SYS_STACK_SIZE - stack_free);
#endif

} /* util_print_sys_info */

#ifdef CONFIG_FS
/*
 * util_print_sys_info_buffer
 * @buffer: File system buffer in which system information is needed to be
 *  added.
 * This function prints generalized information about the operating system in
 * the given file system buffer.
 */
int32_t util_print_sys_info_buffer(FS_BUFFER *buffer)
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_free;
    char str[16];
    int32_t status;

    /* Print current system tick. */
    status = fs_buffer_push(buffer, (uint8_t *)"System tick: ", strlen("System tick: "), 0);

    if (status == SUCCESS)
    {
        snprintf(str, sizeof(str), "%lu", (uint32_t)current_system_tick());
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        status = fs_buffer_push(buffer, (uint8_t *)"\r\n", strlen("\r\n"), 0);
    }

    if (status == SUCCESS)
    {
        /* Print table header. */
        status = fs_buffer_push(buffer, (uint8_t *)"Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\r\n", sizeof("Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\r\n") - 1, 0);
    }

    /* Print information about all the tasks in the system. */
    while ((tcb != NULL) && (status == SUCCESS))
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

        snprintf(str, sizeof(str), "%s\t", tcb->name);
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "%lu\t", tcb->stack_size);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "%lu\t", stack_free);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "%lu\t", tcb->stack_size - stack_free);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "%li\t", tcb->status);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "%lu\t", (uint32_t)tcb->scheduled);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "%lu\r\n", (uint32_t)tcb->tick_sleep);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack(tcb);

    snprintf(str, sizeof(str), "SYSTEM\t");
    status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);

    if (status == SUCCESS)
    {
        snprintf(str, sizeof(str), "%lu\t", (uint32_t)SYS_STACK_SIZE);
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        snprintf(str, sizeof(str), "%lu\t", stack_free);
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        snprintf(str, sizeof(str), "%lu\t", SYS_STACK_SIZE - stack_free);
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        snprintf(str, sizeof(str), "-\t-\t-\r\n");
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }
#endif

    /* Return status to the caller. */
    return (status);

} /* util_print_sys_info_buffer */
#endif /* CONFIG_FS */

#endif /* CONFIG_TASK_STATS */
