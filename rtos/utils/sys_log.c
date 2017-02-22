/*
 * sys_log.c
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

#ifndef SYS_LOG_NONE
#include <string.h>

/* For each component we want to enable logging an entry must exist here. */
SYS_LOG_LEVEL log_level[SYS_LOG_MAX];

/*
 * sys_log_init
 * This function will initialize system logging module.
 */
void sys_log_init()
{
    uint32_t i;

    /* Set default level for all the components. */
    for (i = 0; i < SYS_LOG_MAX; i++)
    {
        log_level[i] = SYS_LOG_DEFAULT;
    }

} /* sys_log_init */

/*
 * sys_log
 * @comp_name: Component name.
 * @message: Message to be printed.
 * @...: Arguments to the formated string in the message.
 * This function logs a debug message on the console.
 */
void sys_log(uint8_t *comp_name, const uint8_t *message, ...)
{
    va_list vl;

    /* Arguments start from the format. */
    va_start(vl, message);

    /* Print the component name. */
    printf("[%s] ", comp_name);

    /* Print the given message. */
    vprintf((const char *)message, vl);

    /* Add a new line. */
    printf("\r\n");

    /* Destroy the argument list. */
    va_end(vl);

} /* sys_log */

/*
 * sys_log_hexdump
 * @comp_name: Component name.
 * @message: Message to be printed before the hex.
 * @bytes: Data buffer to be printed as hex.
 * @num_bytes: Number of bytes to be printed.
 * @...: Arguments to the formated string in the message.
 * This function logs a debug message on the console.
 */
void sys_log_hexdump(uint8_t *comp_name, const uint8_t *message, uint8_t *bytes, uint32_t num_bytes, ...)
{
    va_list vl;

    /* Arguments start from the format. */
    va_start(vl, num_bytes);

    /* Print the component name. */
    printf("[%s] ", comp_name);

    /* If we need to print a message before actual bytes. */
    if (message != NULL)
    {
        /* Print the given message. */
        vprintf((const char *)message, vl);
    }

    /* For we have a byte to dump. */
    for ( ; (num_bytes >= 4); bytes += 4, num_bytes -= 4)
    {
        /* Dump a byte on the console. */
        printf("%02X%02X%02X%02X", bytes[0], bytes[1],  bytes[2], bytes[3]);
    }

    /* For we have a byte to dump. */
    for ( ; (num_bytes > 0); bytes++, num_bytes --)
    {
        /* Dump a byte on the console. */
        printf("%02X", *bytes);
    }

    /* Add a new line. */
    printf("\r\n");

    /* Destroy the argument list. */
    va_end(vl);

} /* sys_log_hexdump */

#endif /* SYS_LOG_NONE */