/*
 * sys_log.h
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
#ifndef _SYS_LOG_H_
#define _SYS_LOG_H_

#include <sys_log_config.h>

/* System logging level definitions. */
#define SYS_LOG_NONE            (0x0)
#define SYS_LOG_FUNCTION_CALL   (0x1)
#define SYS_LOG_DEBUG           (0x2)
#define SYS_LOG_INFO            (0x4)
#define SYS_LOG_WARN            (0x8)
#define SYS_LOG_ERROR           (0x10)
#define SYS_LOG_ALL             (0xFF)

/* System logging level type definition. */
typedef uint8_t SYS_LOG_LEVEL;

/* Component ID list. */
enum
{
    SYS_LOG_DEF = 0,
#ifdef CONFIG_MMC
    SYS_LOG_MMC,
#endif /* CONFIG_MMC */
#ifdef CONFIG_ETHERNET
    SYS_LOG_ENC28J60,
#endif /* CONFIG_ETHERNET */
#ifdef CONFIG_FS
    SYS_LOG_FATFS,
#endif /* CONFIG_FS */
#ifdef CONFIG_TFTPS
    SYS_LOG_TFTPS,
#endif /* CONFIG_TFTPS */
#ifdef CONFIG_NET
    SYS_LOG_ROUTE,
    SYS_LOG_ARP,
    SYS_LOG_NET_BUFFER,
    SYS_LOG_NET_CONDITION,
    SYS_LOG_NET_CSUM,
    SYS_LOG_NET_DEVICE,
    SYS_LOG_DHCPC,
    SYS_LOG_DHCP,
    SYS_LOG_ICMP,
    SYS_LOG_IPV4,
    SYS_LOG_NET_PROCESS,
    SYS_LOG_TCP,
    SYS_LOG_UDP,
    SYS_LOG_NET_WORK,
    SYS_LOG_NET,
#endif /* CONFIG_NET */
    SYS_LOG_MAX
} SYS_LOG_ID;

#ifdef SYS_LOG_RUNTIME_UPDATE
/* Export log levels. */
extern SYS_LOG_LEVEL log_level[SYS_LOG_MAX];
#endif /* SYS_LOG_RUNTIME_UPDATE */

/* Helper macro definitions. */
#define SYS_LOG_IP(ip)  (((uint8_t *)&ip)[3]), (((uint8_t *)&ip)[2]), (((uint8_t *)&ip)[1]), (((uint8_t *)&ip)[0])
#ifdef SYS_LOG_ENABLE
#ifdef SYS_LOG_RUNTIME_UPDATE
#define SYS_LOG_LEVEL_SET(component, level)                                                                             \
        (log_level[SYS_LOG_ ## component] = level)
#define SYS_LOG_MSG(component, level, message, ...)                                                                     \
        if ((log_level[SYS_LOG_ ## component] & level) != 0)                                                            \
        {                                                                                                               \
            sys_log((uint8_t *)#component, (uint8_t *)message, __VA_ARGS__);                                            \
        }
#else
#define SYS_LOG_LEVEL_SET(component, level)
#define SYS_LOG_MSG(component, level, message, ...)                                                                     \
        if ((SYS_LOG_LEVEL_ ## component & level) != 0)                                                                 \
        {                                                                                                               \
            sys_log((uint8_t *)#component, (uint8_t *)message, __VA_ARGS__);                                            \
        }
#endif
#define SYS_LOG_FUNCTION_MSG(component, level, message, ...)                                                            \
        SYS_LOG_MSG(component, level, "%s " message, __func__, __VA_ARGS__)
#define SYS_LOG_FUNCTION_ENTRY(component)                                                                               \
        SYS_LOG_MSG(component, SYS_LOG_FUNCTION_CALL, "%s enter", __func__)
#define SYS_LOG_FUNCTION_EXIT_STATUS(component, status)                                                                 \
        if (status != SUCCESS)                                                                                          \
        {                                                                                                               \
            SYS_LOG_MSG(component, SYS_LOG_ERROR, "%s exit %ld", __func__, status);                                     \
        }                                                                                                               \
        else                                                                                                            \
        {                                                                                                               \
            SYS_LOG_MSG(component, SYS_LOG_FUNCTION_CALL, "%s exit %ld", __func__, status);                             \
        }
#define SYS_LOG_FUNCTION_EXIT(component)                                                                                \
        SYS_LOG_MSG(component, SYS_LOG_FUNCTION_CALL, "%s exit", __func__)
#ifdef SYS_LOG_RUNTIME_UPDATE
#define SYS_LOG_HEXDUMP(component, level, message, bytes, num_bytes)                                                    \
        if ((log_level[SYS_LOG_ ## component] & level) != 0)                                                            \
        {                                                                                                               \
            sys_log_hexdump((uint8_t *)#component, (uint8_t *)message, (uint8_t *)bytes, num_bytes);                    \
        }
#define SYS_LOG_FUNCTION_HEXDUMP(component, level, message, bytes, num_bytes)                                           \
        if ((log_level[SYS_LOG_ ## component] & level) != 0)                                                            \
        {                                                                                                               \
            sys_log_hexdump((uint8_t *)#component, (uint8_t *)("%s " message), (uint8_t *)bytes, num_bytes, __func__);  \
        }
#else
#define SYS_LOG_HEXDUMP(component, level, message, bytes, num_bytes)                                                    \
        if ((SYS_LOG_LEVEL_ ## component & level) != 0)                                                                 \
        {                                                                                                               \
            sys_log_hexdump((uint8_t *)#component, (uint8_t *)message, (uint8_t *)bytes, num_bytes);                    \
        }
#define SYS_LOG_FUNCTION_HEXDUMP(component, level, message, bytes, num_bytes)                                           \
        if ((SYS_LOG_LEVEL_ ## component & level) != 0)                                                                 \
        {                                                                                                               \
            sys_log_hexdump((uint8_t *)#component, (uint8_t *)("%s " message), (uint8_t *)bytes, num_bytes, __func__);  \
        }
#endif /* SYS_LOG_RUNTIME_UPDATE */

/* Function prototypes. */
void sys_log_init(void);
void sys_log(uint8_t *, const uint8_t *, ...);
void sys_log_hexdump(uint8_t *, const uint8_t *, uint8_t *, uint32_t, ...);
#else
#define SYS_LOG_MSG(component, level, message, ...)
#define SYS_LOG_FUNCTION_MSG(component, level, message, ...)
#define SYS_LOG_FUNCTION_ENTRY(component)
#define SYS_LOG_FUNCTION_EXIT_STATUS(component, status)
#define SYS_LOG_FUNCTION_EXIT(component)
#define SYS_LOG_HEXDUMP(component, level, message, bytes, num_bytes)
#define SYS_LOG_FUNCTION_HEXDUMP(component, level, message, bytes, num_bytes)
#endif /* SYS_LOG_ENABLE */

#endif
