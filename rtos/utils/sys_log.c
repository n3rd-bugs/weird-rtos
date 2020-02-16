/*
 * sys_log.c
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

#ifdef SYS_LOG_ENABLE
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <rtl.h>
#include <serial.h>

#ifdef SYS_LOG_RUNTIME_UPDATE
/* For each component we want to enable logging an entry must exist here. */
SYS_LOG_LEVEL log_level[SYS_LOG_MAX];
#endif /* SYS_LOG_RUNTIME_UPDATE */

/*
 * sys_log_init
 * This function will initialize system logging module.
 */
void sys_log_init(void)
{
#ifdef SYS_LOG_RUNTIME_UPDATE
    /* Set default level for all the components. */
    log_level[SYS_LOG_DEF]              = SYS_LOG_LEVEL_DEF;
#ifdef IO_MMC
    log_level[SYS_LOG_MMC]              = SYS_LOG_LEVEL_MMC;
#endif /* IO_MMC */
#ifdef IO_ETHERNET
    log_level[SYS_LOG_ENC28J60]         = SYS_LOG_LEVEL_ENC28J60;
#endif /* IO_ETHERNET */
#ifdef CONFIG_FS
    log_level[SYS_LOG_FATFS]            = SYS_LOG_LEVEL_FATFS;
#endif /* CONFIG_FS */
#ifdef CONFIG_TFTPS
    log_level[SYS_LOG_TFTPS]            = SYS_LOG_LEVEL_TFTPS;
#endif /* CONFIG_TFTPS */
#ifdef CONFIG_NET
    log_level[SYS_LOG_ROUTE]            = SYS_LOG_LEVEL_ROUTE;
    log_level[SYS_LOG_ARP]              = SYS_LOG_LEVEL_ARP;
    log_level[SYS_LOG_NET_BUFFER]       = SYS_LOG_LEVEL_NET_BUFFER;
    log_level[SYS_LOG_NET_CONDITION]    = SYS_LOG_LEVEL_NET_CONDITION;
    log_level[SYS_LOG_NET_CSUM]         = SYS_LOG_LEVEL_NET_CSUM;
    log_level[SYS_LOG_NET_DEVICE]       = SYS_LOG_LEVEL_NET_DEVICE;
    log_level[SYS_LOG_DHCPC]            = SYS_LOG_LEVEL_DHCPC;
    log_level[SYS_LOG_DHCP]             = SYS_LOG_LEVEL_DHCP;
    log_level[SYS_LOG_ICMP]             = SYS_LOG_LEVEL_ICMP;
    log_level[SYS_LOG_IPV4]             = SYS_LOG_LEVEL_IPV4;
    log_level[SYS_LOG_NET_PROCESS]      = SYS_LOG_LEVEL_NET_PROCESS;
    log_level[SYS_LOG_TCP]              = SYS_LOG_LEVEL_TCP;
    log_level[SYS_LOG_UDP]              = SYS_LOG_LEVEL_UDP;
    log_level[SYS_LOG_NET_WORK]         = SYS_LOG_LEVEL_NET_WORK;
    log_level[SYS_LOG_NET]              = SYS_LOG_LEVEL_NET;
#endif /* CONFIG_NET */
#endif /* SYS_LOG_RUNTIME_UPDATE */
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

#ifndef IO_SERIAL
    /* Removed some compiler warnings. */
    UNUSED_PARAM(comp_name);
#endif

    /* Arguments start from the format. */
    va_start(vl, message);

#ifdef IO_SERIAL
    /* Print the component name. */
    io_puts("[", -1);
    io_puts((const char *)comp_name, -1);
    io_puts("]", -1);

    /* Print the given message. */
    vprintf((const char *)message, vl);

    /* Add a new line. */
    io_puts("\r\n", -1);
#endif /* IO_SERIAL */

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
 * This function logs a debug message on the console along with HEX dump of
 * provided memory region.
 */
void sys_log_hexdump(uint8_t *comp_name, const uint8_t *message, uint8_t *bytes, uint32_t num_bytes, ...)
{
    va_list vl;

#ifndef IO_SERIAL
    /* Removed some compiler warnings. */
    UNUSED_PARAM(comp_name);
    UNUSED_PARAM(message);
    UNUSED_PARAM(bytes);
#endif

    /* Arguments start from the format. */
    va_start(vl, num_bytes);

#ifdef IO_SERIAL
    /* Print the component name. */
    io_puts("[", -1);
    io_puts((const char *)comp_name, -1);
    io_puts("]", -1);

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
    io_puts("\r\n", -1);
#endif /* IO_SERIAL */

    /* Destroy the argument list. */
    va_end(vl);

} /* sys_log_hexdump */

#endif /* SYS_LOG_ENABLE */
