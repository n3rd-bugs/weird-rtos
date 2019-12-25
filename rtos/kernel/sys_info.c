/*
 * sys_info.c
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
#include <kernel.h>
#include <sys_info.h>
#ifdef CONFIG_SERIAL
#include <serial.h>
#endif /* CONFIG_SERIAL */

#ifdef TASK_STATS
#ifdef CONFIG_FS
#include <fs.h>
#endif /* CONFIG_FS */
#include <string.h>
#include <rtl.h>

/* String definitions. */
static const char __sys_info_sys_tick[] PROGMEM = "System tick: ";
static const char __sys_info_name_pri[] PROGMEM = "Name\tPri.\t";
static const char __sys_info_st_sf[] PROGMEM = "S(T)\tS(F)\t";
static const char __sys_info_sm_status[] PROGMEM = "S(M)\tStatus\t";
#ifdef TASK_USAGE
static const char __sys_info_cpu[] PROGMEM = "CPU(%)\t";
#endif /* TASK_USAGE */
#ifdef CONFIG_SERIAL
static const char __sys_info_running[] PROGMEM = "\t<Running>";
#endif /* CONFIG_SERIAL */
static const char __sys_info_system[] PROGMEM = "SYSTEM\t-\t";
static const char __sys_info_dash[] PROGMEM = "\t-";
static const char __sys_info_tab[] PROGMEM = "\t";
static const char __sys_info_new_line[] PROGMEM = "\r\n";
#ifdef CONFIG_SLEEP
static const char __sys_info_nt_st[] PROGMEM = "n(T)\ts(T)\r\n";
#ifdef CONFIG_SERIAL
static const char __sys_info_dash_dash_dash[] PROGMEM = "\t-\t-\t-\r\n";
#endif /* CONFIG_SERIAL */
static const char __sys_info_dash_dash[] PROGMEM = "\t-\t-\r\n";
#else
static const char __sys_info_nt_st[] PROGMEM = "n(T)\r\n";
#ifdef CONFIG_SERIAL
static const char __sys_info_dash_dash_dash[] PROGMEM = "\t-\t-\r\n";
#endif /* CONFIG_SERIAL */
static const char __sys_info_dash_dash[] PROGMEM = "\t-\r\n";
#endif /* CONFIG_SLEEP */

#ifdef TASK_USAGE
/* Number of ticks system context was active. */
static uint64_t sys_total_active_ticks;

/* Last tick at which we switched to system context. */
static uint64_t sys_last_active_tick;

/* Tick at which we reset the usage statistics. */
static uint64_t sys_clock_base;

#ifdef TASK_USAGE_RETAIN
/* Last number of ticks system context was active. */
static uint64_t last_sys_total_active_ticks;

/* Last session's interval in ticks. */
static uint64_t last_session_ticks;
#endif /*TASK_USAGE_RETAIN*/

/*
 * mark_task_entry
 * This function marks the entry of a task if we are in one.
 */
void mark_task_entry(void)
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
void mark_task_exit(void)
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
void usage_reset(void)
{
    TASK *tcb = sch_task_list.head;

#ifdef TASK_USAGE_RETAIN
    /* Save the last session's interval. */
    last_session_ticks =  current_hardware_tick() - sys_clock_base;
#endif /*TASK_USAGE_RETAIN*/

    /* Pick the new base clock. */
    sys_clock_base = current_hardware_tick();

    /* Lock the schedule. */
    scheduler_lock();

    /* Traverse the list of all the tasks. */
    while (tcb != NULL)
    {
#ifdef TASK_USAGE_RETAIN
        /* Save old total ticks. */
        tcb->last_total_active_ticks = tcb->total_active_ticks;
#endif /*TASK_USAGE_RETAIN*/

        /* Reset total active ticks for this task. */
        tcb->total_active_ticks = 0;

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef TASK_USAGE_RETAIN
    /* Save the last total system active ticks. */
    last_sys_total_active_ticks = sys_total_active_ticks;
#endif /*TASK_USAGE_RETAIN*/

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
#ifndef TASK_USAGE_RETAIN
    INT_LVL interrupt_level;
#endif /* TASK_USAGE_RETAIN */

    /* If we have task. */
    if (task != NULL)
    {
#ifdef TASK_USAGE_RETAIN
        /* Pick the last session's total number of active ticks. */
        usage = task->last_total_active_ticks;
#else
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
#endif /*TASK_USAGE_RETAIN*/
    }
    else
    {
#ifdef TASK_USAGE_RETAIN
        /* Pick the last session's ticks for which we were active in system context. */
        usage = last_sys_total_active_ticks;
#else
        /* Pick the ticks we were active in system context. */
        usage = sys_total_active_ticks;
#endif /*TASK_USAGE_RETAIN*/
    }

    /* Calculate and scale the usage. */
#ifdef TASK_USAGE_RETAIN
    usage = ((usage * scale) / (last_session_ticks));
#else
    usage = ((usage * scale) / (current_hardware_tick() - sys_clock_base));
#endif /*TASK_USAGE_RETAIN*/

    /* Return the usage to the caller. */
    return (usage);

} /* usage_calculate */
#endif /* TASK_USAGE */

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
    while (tcb->stack_start[free] == TASK_STACK_PATTERN)
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
uint32_t util_system_calc_free_stack(void)
{
    uint32_t free = 0;

    /* Calculate the number of bytes intact on the system stack. */
    while (SYSTEM_STACK[free] == TASK_STACK_PATTERN)
    {
        free ++;
    }

    /* Return number of free bytes on the stack. */
    return (free);

} /* util_system_calc_free_stack */
#endif

#ifdef CONFIG_SERIAL
/*
 * util_print_sys_info
 * This function prints generalized information about the operating system.
 */
void util_print_sys_info(void)
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_free;
    char str[16];
#ifdef TASK_USAGE
    uint32_t usage;
#endif /* TASK_USAGE */

#ifdef CONFIG_SLEEP
    /* Print current system tick. */
    P_STR_NCPY(str, __sys_info_sys_tick, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    rtl_ultoa_b10(current_system_tick(), (uint8_t*)str);
    fs_puts(debug_fd, (uint8_t *)str, -1);
    P_STR_NCPY(str, __sys_info_new_line, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
#endif /* CONFIG_SLEEP */

    /* Print table header. */
    P_STR_NCPY(str, __sys_info_name_pri, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    P_STR_NCPY(str, __sys_info_st_sf, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    P_STR_NCPY(str, __sys_info_sm_status, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
#ifdef TASK_USAGE
    P_STR_NCPY(str, __sys_info_cpu, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
#endif /* TASK_USAGE */
    P_STR_NCPY(str, __sys_info_nt_st, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);

    /* Print information about all the tasks in the system. */
    while (tcb != NULL)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

#ifdef TASK_USAGE
        /* Calculate % CPU usage for this task. */
        usage = (uint32_t)usage_calculate(tcb, 100);
#endif /* TASK_USAGE */

        /* Print task information. */
        P_STR_NCPY(str, tcb->name, (P_STR_LEN(tcb->name) > sizeof(str)) ? (sizeof(str) - 1) : P_STR_LEN(tcb->name));
        str[(P_STR_LEN(tcb->name) > sizeof(str)) ? (sizeof(str) - 1) : P_STR_LEN(tcb->name)] = '\0';
        fs_puts(debug_fd, (uint8_t *)str, -1);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(tcb->priority, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(tcb->stack_size, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(stack_free, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(tcb->stack_size - stack_free, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(tcb->state, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
#ifdef TASK_USAGE
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(usage, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
#endif /* TASK_USAGE */
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(tcb->scheduled, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
#ifdef CONFIG_SLEEP
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);
        rtl_ultoa_b10(tcb->tick_sleep, (uint8_t*)str);
        fs_puts(debug_fd, (uint8_t *)str, -1);
#endif /* CONFIG_SLEEP */
        if (tcb == get_current_task())
        {
            P_STR_NCPY(str, __sys_info_running, sizeof(str));
            fs_puts(debug_fd, (uint8_t *)str, -1);
        }
        P_STR_NCPY(str, __sys_info_new_line, sizeof(str));
        fs_puts(debug_fd, (uint8_t *)str, -1);

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack();

#ifdef TASK_USAGE
    /* Calculate % CPU usage for system. */
    usage = (uint32_t)usage_calculate(NULL, 100);
#endif /* TASK_USAGE */

    /* Print system stack information. */
    P_STR_NCPY(str, __sys_info_system, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    rtl_ultoa_b10((uint32_t)SYS_STACK_SIZE, (uint8_t*)str);
    fs_puts(debug_fd, (uint8_t *)str, -1);
    P_STR_NCPY(str, __sys_info_tab, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    rtl_ultoa_b10(stack_free, (uint8_t*)str);
    fs_puts(debug_fd, (uint8_t *)str, -1);
    P_STR_NCPY(str, __sys_info_tab, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    rtl_ultoa_b10(SYS_STACK_SIZE - stack_free, (uint8_t*)str);
    fs_puts(debug_fd, (uint8_t *)str, -1);
    P_STR_NCPY(str, __sys_info_dash, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
#ifdef TASK_USAGE
    P_STR_NCPY(str, __sys_info_tab, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
    rtl_ultoa_b10(usage, (uint8_t*)str);
    fs_puts(debug_fd, (uint8_t *)str, -1);
#endif /* TASK_USAGE */
    P_STR_NCPY(str, __sys_info_dash_dash, sizeof(str));
    fs_puts(debug_fd, (uint8_t *)str, -1);
#endif /* SYS_STACK_SIZE */

} /* util_print_sys_info */
#endif /* CONFIG_SERIAL */

#ifdef CONFIG_SERIAL
/*
 * util_print_sys_info_assert
 * This function prints generalized information about the operating system
 * on the serial port in assert mode.
 */
void util_print_sys_info_assert(void)
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_free;
    char str[16];

#ifdef CONFIG_SLEEP
    /* Print current system tick. */
    P_STR_NCPY(str, __sys_info_sys_tick, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    rtl_ultoa_b10(current_system_tick(), (uint8_t*)str);
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_new_line, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
#endif /* CONFIG_SLEEP */

    /* Print table header. */
    P_STR_NCPY(str, __sys_info_name_pri, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_st_sf, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_sm_status, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_nt_st, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);

    /* Print information about all the tasks in the system. */
    while (tcb != NULL)
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

        P_STR_NCPY(str, tcb->name, (P_STR_LEN(tcb->name) > sizeof(str)) ? (sizeof(str) - 1) : P_STR_LEN(tcb->name));
        str[(P_STR_LEN(tcb->name) > sizeof(str)) ? (sizeof(str) - 1) : P_STR_LEN(tcb->name)] = '\0';
        serial_assert_puts((uint8_t *)str, 0);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(tcb->priority, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(tcb->stack_size, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(stack_free, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(tcb->stack_size - stack_free, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(tcb->state, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(tcb->scheduled, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
#ifdef CONFIG_SLEEP
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);
        rtl_ultoa_b10(tcb->tick_sleep, (uint8_t*)str);
        serial_assert_puts((uint8_t *)str, 0);
#endif /* CONFIG_SLEEP */
        if (tcb == get_current_task())
        {
            P_STR_NCPY(str, __sys_info_running, sizeof(str));
            serial_assert_puts((uint8_t *)str, 0);
        }
        P_STR_NCPY(str, __sys_info_new_line, sizeof(str));
        serial_assert_puts((uint8_t *)str, 0);

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack();

    P_STR_NCPY(str, __sys_info_system, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    rtl_ultoa_b10((uint32_t)SYS_STACK_SIZE, (uint8_t*)str);
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_tab, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    rtl_ultoa_b10(stack_free, (uint8_t*)str);
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_tab, sizeof(str));
    serial_assert_puts((uint8_t *)str, 0);
    rtl_ultoa_b10(SYS_STACK_SIZE - stack_free, (uint8_t*)str);
    serial_assert_puts((uint8_t *)str, 0);
    P_STR_NCPY(str, __sys_info_dash_dash_dash, sizeof(str));
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
int32_t util_print_sys_info_buffer(FS_BUFFER_LIST *buffer)
{
    /* Get the first task. */
    TASK *tcb = sch_task_list.head;
    uint32_t stack_free;
    char str[16];
    int32_t status;
#ifdef TASK_USAGE
    uint32_t usage;
#endif /* TASK_USAGE */

#ifdef CONFIG_SLEEP
    /* Print current system tick. */
    P_STR_NCPY(str, __sys_info_sys_tick, sizeof(str));
    status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);

    if (status == SUCCESS)
    {
        rtl_ultoa_b10(current_system_tick(), (uint8_t*)str);
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        P_STR_NCPY(str, __sys_info_new_line, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
#endif /* CONFIG_SLEEP */
    {
        /* Print table header. */
        P_STR_NCPY(str, __sys_info_name_pri, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_st_sf, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_sm_status, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

#ifdef TASK_USAGE
        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_cpu, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }
#endif

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_nt_st, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }
    }

    /* Print information about all the tasks in the system. */
    while ((tcb != NULL) && (status == SUCCESS))
    {
        /* Calculate number of bytes still intact on the task's stack. */
        stack_free = util_task_calc_free_stack(tcb);

        P_STR_NCPY(str, tcb->name, (P_STR_LEN(tcb->name) > sizeof(str)) ? (sizeof(str) - 1) : P_STR_LEN(tcb->name));
        str[(P_STR_LEN(tcb->name) > sizeof(str)) ? (sizeof(str) - 1) : P_STR_LEN(tcb->name)] = '\0';
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(tcb->priority, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(tcb->stack_size, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(stack_free, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(tcb->stack_size - stack_free, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(tcb->state, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

#ifdef TASK_USAGE
        if (status == SUCCESS)
        {
            /* Calculate % CPU usage for this task. */
            usage = (uint32_t)usage_calculate(tcb, 100);

            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);

            if (status == SUCCESS)
            {
                rtl_ultoa_b10(usage, (uint8_t*)str);
                status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
            }
        }
#endif

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(tcb->scheduled, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

#ifdef CONFIG_SLEEP
        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_tab, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(tcb->tick_sleep, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }
#endif /* CONFIG_SLEEP */

        if (status == SUCCESS)
        {
            P_STR_NCPY(str, __sys_info_new_line, sizeof(str));
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }

        /* Get the next task. */
        tcb = tcb->next_global;
    }

#ifdef SYS_STACK_SIZE
    /* Get number of bytes free on the system stack. */
    stack_free = util_system_calc_free_stack();

    P_STR_NCPY(str, __sys_info_system, sizeof(str));
    status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);

    if (status == SUCCESS)
    {
        rtl_ultoa_b10((uint32_t)SYS_STACK_SIZE, (uint8_t*)str);
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        rtl_ultoa_b10(stack_free, (uint8_t*)str);
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        rtl_ultoa_b10(SYS_STACK_SIZE - stack_free, (uint8_t*)str);
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

    if (status == SUCCESS)
    {
        P_STR_NCPY(str, __sys_info_dash, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }

#ifdef TASK_USAGE
    if (status == SUCCESS)
    {
        /* Calculate % CPU usage for this task. */
        usage = (uint32_t)usage_calculate(NULL, 100);

        P_STR_NCPY(str, __sys_info_tab, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);

        if (status == SUCCESS)
        {
            rtl_ultoa_b10(usage, (uint8_t*)str);
            status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
        }
    }
#endif

    if (status == SUCCESS)
    {
        P_STR_NCPY(str, __sys_info_dash_dash, sizeof(str));
        status = fs_buffer_list_push(buffer, (uint8_t *)str, strlen(str), 0);
    }
#endif

    /* Return status to the caller. */
    return (status);

} /* util_print_sys_info_buffer */
#endif /* CONFIG_FS */

#endif /* TASK_STATS */
