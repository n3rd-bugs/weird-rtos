/*
 * assert.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#include <kernel.h>
#include <sys_info.h>
#include <serial.h>
#include <string.h>
#include <stdlib.h>

/*
 * system_assert
 * @code: This can be used to identify the issue.
 * @file: File from where this assert was raised.
 * @line: Line number from which this assert was raised.
 * @task: The task context from which this was called.
 * This function will check if any of the allocated memory has overflow.
 */
void system_assert(int32_t code, char *file, uint32_t line, TASK *task)
{
#ifdef CONFIG_SERIAL
    uint8_t line_buf[12];
#endif

    /* Remove some compiler warnings. */
    UNUSED_PARAM(code);
    UNUSED_PARAM(task);
#if (!defined(ASSERT_FILE_INFO) || !defined(CONFIG_SERIAL))
    UNUSED_PARAM(file);
    UNUSED_PARAM(line);
#endif

    /* Disable system interrupts. */
    DISABLE_INTERRUPTS();

#ifdef CONFIG_SERIAL
#ifdef ASSERT_FILE_INFO
    /* Put filename. */
    serial_assert_puts((uint8_t *)file, 0);

    /* Put colon. */
    serial_assert_puts((uint8_t *)":", 0);

    /* Print line number. */
    itoa((int)line, (char *)line_buf, 10);
    serial_assert_puts(line_buf, 0);

    /* Put line terminator. */
    serial_assert_puts((uint8_t *)"\r\n", 0);
#endif /* ASSERT_FILE_INFO */

#ifdef CONFIG_TASK_STATS
    /* Print current system information. */
    util_print_sys_info_assert();
#endif /* CONFIG_TASK_STATS */
#endif /* CONFIG_SERIAL */

    /* For now there are no recovery mechanisms defined. */
    while (1)
    {
        ;
    }

} /* system_assert */
