/*
 * sys_log.h
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
#ifndef _SYS_LOG_H_
#define _SYS_LOG_H_

/* System log configuration. */
//#define SYS_LOG_NONE

/* System logging level definitions. */
#define SYS_LOG_FUNCTION_CALL   (0x01)
#define SYS_LOG_DEBUG           (0x02)
#define SYS_LOG_INFO            (0x04)
#define SYS_LOG_WARN            (0x08)
#define SYS_LOG_ERROR           (0x10)
#define SYS_LOG_ALL             (0xFF)
#define SYS_LOG_DEFAULT         ((SYS_LOG_INFO) | (SYS_LOG_WARN) | (SYS_LOG_ERROR))

/* System logging level type definition. */
typedef uint8_t SYS_LOG_LEVEL;

/* Component ID list. */
enum
{
    SYS_LOG_DEF = 0,
#ifdef CONFIG_MMC
    SYS_LOG_MMC,
#endif
#ifdef CONFIG_ETHERNET
    SYS_LOG_ENC28J60,
#endif
    SYS_LOG_MAX
} SYS_LOG_ID;

/* Export log levels. */
extern SYS_LOG_LEVEL log_level[SYS_LOG_MAX];

/* Helper macro definitions. */
#ifdef SYS_LOG_NONE
#define SYS_LOG_MSG(component, level, message, ...)
#define SYS_LOG_FUNTION_MSG(component, level, message, ...)
#define SYS_LOG_FUNTION_ENTRY(component)
#define SYS_LOG_FUNTION_EXIT_STATUS(component, status)
#define SYS_LOG_FUNTION_EXIT(component)
#define SYS_LOG_HEXDUMP(component, level, message, bytes, num_bytes)
#define SYS_LOG_FUNTION_HEXDUMP(component, level, message, bytes, num_bytes)
#else
#define SYS_LOG_MSG(component, level, message, ...)                                                                     \
        if ((log_level[SYS_LOG_ ## component] & level) != 0)                                                            \
        {                                                                                                               \
            sys_log((uint8_t *)#component, (uint8_t *)message, __VA_ARGS__);                                            \
        }
#define SYS_LOG_FUNTION_MSG(component, level, message, ...)                                                             \
        SYS_LOG_MSG(component, level, "%s " message, __func__, __VA_ARGS__)
#define SYS_LOG_FUNTION_ENTRY(component)                                                                                \
        SYS_LOG_MSG(component, SYS_LOG_FUNCTION_CALL, "%s enter", __func__)
#define SYS_LOG_FUNTION_EXIT_STATUS(component, status)                                                                  \
        SYS_LOG_MSG(component, SYS_LOG_FUNCTION_CALL, "%s exit %ld", __func__, status)
#define SYS_LOG_FUNTION_EXIT(component)                                                                                 \
        SYS_LOG_MSG(component, SYS_LOG_FUNCTION_CALL, "%s exit", __func__)
#define SYS_LOG_HEXDUMP(component, level, message, bytes, num_bytes)                                                    \
        if ((log_level[SYS_LOG_ ## component] & level) != 0)                                                            \
        {                                                                                                               \
            sys_log_hexdump((uint8_t *)#component, (uint8_t *)message, (uint8_t *)bytes, num_bytes);                    \
        }
#define SYS_LOG_FUNTION_HEXDUMP(component, level, message, bytes, num_bytes)                                            \
        if ((log_level[SYS_LOG_ ## component] & level) != 0)                                                            \
        {                                                                                                               \
            sys_log_hexdump((uint8_t *)#component, (uint8_t *)("%s " message), (uint8_t *)bytes, num_bytes, __func__);  \
        }
#endif

/* Function prototypes. */
void sys_log_init();
void sys_log(uint8_t *, const uint8_t *, ...);
void sys_log_hexdump(uint8_t *, const uint8_t *, uint8_t *, uint32_t, ...);

#endif
