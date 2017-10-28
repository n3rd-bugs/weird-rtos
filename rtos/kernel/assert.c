/*
 * assert.c
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
#include <sys_info.h>
#include <string.h>
#include <stdlib.h>
#ifdef CONFIG_SERIAL
#include <serial.h>
#endif /* CONFIG_SERIAL */

/*
 * system_assert
 * @code: This can be used to identify the issue.
 * @file: File from where this assert was raised.
 * @line: Line number from which this assert was raised.
 * @task: The task context from which this was called.
 * This function will halt the system and print the file/line and task
 * information if required.
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

#ifdef TASK_STATS
    /* Print current system information. */
    util_print_sys_info_assert();
#endif /* TASK_STATS */
#endif /* CONFIG_SERIAL */

    /* For now there are no recovery mechanisms defined. */
    while (1)
    {
        ;
    }

} /* system_assert */
