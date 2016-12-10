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
 * (in any form) the author will not be liable for any legal charges.
 */

#include <os.h>
#include <sys_info.h>

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
    /* Remove some compiler warnings. */
    UNUSED_PARAM(code);
    UNUSED_PARAM(file);
    UNUSED_PARAM(line);
    UNUSED_PARAM(task);

    /* Disable system interrupts. */
    DISABLE_INTERRUPTS();

#ifdef ASSERT_FILE_INFO
    /* Print file information. */
    printf("%s:%ld\r\n", file, line);
#endif

#ifdef CONFIG_TASK_STATS
    /* Print current system information. */
    util_print_sys_info();
#endif

    /* For now there are no recovery mechanisms defined. */
    while (1)
    {
        ;
    }

} /* system_assert */
