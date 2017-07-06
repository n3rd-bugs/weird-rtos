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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <stdio.h>
#include <kernel.h>
#include <sys_info.h>
#include <serial.h>

#ifdef CONFIG_TASK_STATS
#ifdef CONFIG_FS
#include <fs.h>
#include <string.h>
#endif

#ifdef CONFIG_TASK_USAGE
/* Number of ticks system context was active. */
static uint64_t sys_total_active_ticks;

/* Last tick at which we switched to system context. */
static uint64_t sys_last_active_tick;

/* Tick at which we reset the usage statistics. */
static uint64_t sys_clock_base;

/*
 * mark_task_entry
 * This function marks the entry of a task if we are in one.
 */
void mark_task_entry()
{
    /* If we do have a current task. */
    if (current_task != NULL)
    {
        /* Save the tick at which this task was scheduled. */
        current_task->last_active_tick = current_hardware_tick();

        /* Increment the number of ticks we were in system context. */
        sys_total_active_ticks += (current_task->last_active_tick - sys_last_active_tick);
    }

} /* mark_task_entry */

/*
 * mark_task_exit
 * This function marks the exit of current task if we are in one.
 */
void mark_task_exit()
{
    /* If we do have a current task. */
    if (current_task != NULL)
    {
        /* Save the tick at which we entered the system context. */
        sys_last_active_tick = current_hardware_tick();

        /* Increment the number of ticks we were in this task's context. */
        current_task->total_active_ticks += sys_last_active_tick - current_task->last_active_tick;
    }

} /* mark_task_exit */

/*
 * usage_reset
 * This function reset the CPU usage statistics for all the tasks and system.
 */
void usage_reset()
{
    TASK *tcb = sch_task_list.head;

    /* Pick the new base clock. */
    sys_clock_base = current_hardware_tick();

    /* Lock the schedule. */
    scheduler_lock();

    /* Traverse the list of all the tasks. */
    while (tcb != NULL)
    {
        /* Reset total active ticks for this task. */
        tcb->total_active_ticks = 0;

        /* Get the next task. */
        tcb = tcb->next_global;
    }

    /* Reset total system active ticks. */
    sys_total_active_ticks = 0;

    /* If we are in a task's context. */
    if (get_current_task() != NULL)
    {
        /* Mark this tick as task's last active tick. */
        get_current_task()->last_active_tick = sys_clock_base;
    }
    else
    {
        /* Mark this tick as system's last active tick. */
        sys_last_active_tick = sys_clock_base;
    }

    /* Unlock the schedule. */
    scheduler_unlock();

} /* usage_reset */

/*
 * usage_calculate
 * @task: Task control block for which CPU usage is required, if null system
 *  CPU usage will be returned.
 * @scale: Scale in which CPU usage is required.
 * @return: Scaled CPU usage of current task.
 * This function calculates and return the given task's CPU usage.
 */
uint64_t usage_calculate(TASK *task, uint64_t scale)
{
    uint64_t usage;
    INT_LVL interrupt_level;

    /* If we have task. */
    if (task != NULL)
    {
        /* Pick the total number of active ticks. */
        usage = task->total_active_ticks;

        /* If we are in this task. */
        if (current_task == task)
        {
            /* Disable interrupts. */
            interrupt_level = GET_INTERRUPT_LEVEL();
            DISABLE_INTERRUPTS();

            /* Add current usage. */
            usage += (current_hardware_tick() - current_task->last_active_tick);

            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);
        }
    }
    else
    {
        /* Pick the ticks we were active in system context. */
        usage = sys_total_active_ticks;
    }

    /* Calculate and scale the usage. */
    usage = ((usage * scale) / (current_hardware_tick() - sys_clock_base));

    /* Return the usage to the caller. */
    return (usage);

} /* usage_calculate */
#endif /* CONFIG_TASK_USAGE */

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
 * This function returns the number of bytes free on the system stack.
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
#ifdef CONFIG_TASK_USAGE
    uint32_t usage;
#endif /* CONFIG_TASK_USAGE */

    /* Print current system tick. */
    printf("System tick: %lu\r\n", (uint32_t)current_system_tick());

    /* Print table header. */
    printf("Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)");
#ifdef CONFIG_TASK_USAGE
    printf("\tCPU(%%)");
#endif /* CONFIG_TASK_USAGE */
    printf("\r\n");

    /* Print information about all the tasks in the system. */
    while (tcb != NULL)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

#ifdef CONFIG_TASK_USAGE
        /* Calculate % CPU usage for this task. */
        usage = (uint32_t)usage_calculate(tcb, 100);
#endif /* CONFIG_TASK_USAGE */

        /* Print task information. */
        printf("%s\t%lu\t%lu\t%lu\t%li\t%lu\t%lu", tcb->name, tcb->stack_size, stack_free, tcb->stack_size - stack_free, tcb->status, tcb->scheduled, tcb->tick_sleep);
#ifdef CONFIG_TASK_USAGE
        printf("\t%lu", usage);
#endif /* CONFIG_TASK_USAGE */
        printf("%s\r\n", (tcb == get_current_task()) ? "\t<Running>" : "");

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack();

#ifdef CONFIG_TASK_USAGE
    /* Calculate % CPU usage for system. */
    usage = (uint32_t)usage_calculate(NULL, 100);
#endif /* CONFIG_TASK_USAGE */

    /* Print system stack information. */
    printf("SYSTEM\t%lu\t%lu\t%lu\t-\t-\t-\t%lu\r\n",
           (uint32_t)SYS_STACK_SIZE,
           stack_free,
           SYS_STACK_SIZE - stack_free,
           usage);
#endif /* SYS_STACK_SIZE */

} /* util_print_sys_info */

#ifdef CONFIG_SERIAL
/*
 * util_print_sys_info_assert
 * This function prints generalized information about the operating system
 * on the serial port in assert mode.
 */
void util_print_sys_info_assert()
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_free;
    char str[16];

    /* Print current system tick. */
    serial_assert_puts((uint8_t *)"System tick: ", 0);
    snprintf(str, sizeof(str), "%lu", (uint32_t)current_system_tick());
    serial_assert_puts((uint8_t *)str, 0);
    serial_assert_puts((uint8_t *)"\r\n", 0);

    /* Print table header. */
    serial_assert_puts((uint8_t *)"Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\r\n", 0);

    /* Print information about all the tasks in the system. */
    while (tcb != NULL)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

        snprintf(str, sizeof(str), "%s\t", tcb->name);
        serial_assert_puts((uint8_t *)str, 0);
        snprintf(str, sizeof(str), "%lu\t", tcb->stack_size);
        serial_assert_puts((uint8_t *)str, 0);
        snprintf(str, sizeof(str), "%lu\t", stack_free);
        serial_assert_puts((uint8_t *)str, 0);
        snprintf(str, sizeof(str), "%lu\t", tcb->stack_size - stack_free);
        serial_assert_puts((uint8_t *)str, 0);
        snprintf(str, sizeof(str), "%li\t", tcb->status);
        serial_assert_puts((uint8_t *)str, 0);
        snprintf(str, sizeof(str), "%lu\t", (uint32_t)tcb->scheduled);
        serial_assert_puts((uint8_t *)str, 0);
        snprintf(str, sizeof(str), "%lu\r\n", (uint32_t)tcb->tick_sleep);
        serial_assert_puts((uint8_t *)str, 0);

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack(tcb);

    snprintf(str, sizeof(str), "SYSTEM\t");
    serial_assert_puts((uint8_t *)str, 0);
    snprintf(str, sizeof(str), "%lu\t", (uint32_t)SYS_STACK_SIZE);
    serial_assert_puts((uint8_t *)str, 0);
    snprintf(str, sizeof(str), "%lu\t", stack_free);
    serial_assert_puts((uint8_t *)str, 0);
    snprintf(str, sizeof(str), "%lu\t", SYS_STACK_SIZE - stack_free);
    serial_assert_puts((uint8_t *)str, 0);
    snprintf(str, sizeof(str), "-\t-\t-\r\n");
    serial_assert_puts((uint8_t *)str, 0);
#endif /* SYS_STACK_SIZE */

} /* util_print_sys_info_assert */
#endif /* CONFIG_SERIAL */

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
#ifdef CONFIG_TASK_USAGE
    uint32_t usage;
#endif /* CONFIG_TASK_USAGE */

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
#ifdef CONFIG_TASK_USAGE
        /* Print table header. */
        status = fs_buffer_push(buffer, (uint8_t *)"Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\tCPU(%)\r\n", sizeof("Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\tCPU(%)\r\n") - 1, 0);
#else
        /* Print table header. */
        status = fs_buffer_push(buffer, (uint8_t *)"Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\r\n", sizeof("Name\tTotal\tFree\tMin.\tStatus\tn(T)\ts(T)\r\n") - 1, 0);
#endif
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
            snprintf(str, sizeof(str), "%lu", (uint32_t)tcb->tick_sleep);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

#ifdef CONFIG_TASK_USAGE
        if (status == SUCCESS)
        {
            /* Calculate % CPU usage for this task. */
            usage = (uint32_t)usage_calculate(tcb, 100);

            snprintf(str, sizeof(str), "\t%lu", usage);
            status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
        }
#endif

        if (status == SUCCESS)
        {
            snprintf(str, sizeof(str), "\r\n");
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
        snprintf(str, sizeof(str), "-\t-\t-");
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

#ifdef CONFIG_TASK_USAGE
    if (status == SUCCESS)
    {
        /* Calculate % CPU usage for this task. */
        usage = (uint32_t)usage_calculate(NULL, 100);

        snprintf(str, sizeof(str), "\t%lu", usage);
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }
#endif

    if (status == SUCCESS)
    {
        snprintf(str, sizeof(str), "\r\n");
        status = fs_buffer_push(buffer, (uint8_t *)str, strlen(str), 0);
    }
#endif

    /* Return status to the caller. */
    return (status);

} /* util_print_sys_info_buffer */
#endif /* CONFIG_FS */

#endif /* CONFIG_TASK_STATS */
