/*
 * sleep.h
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
#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <kernel.h>

#ifdef CONFIG_SLEEP

/* Function prototypes. */
void sleep_process_system_tick(void);
void sleep_add_to_list(TASK *, uint32_t);
void sleep_remove_from_list(TASK *);
void sleep_ticks(uint32_t);
void sleep_hw_ticks(uint64_t);
#define sleep_ms(ms)                sleep_ticks(MS_TO_TICK((ms)))
#define sleep_fms(ms)               sleep_ticks((MS_TO_TICK((ms)) > 0) ? MS_TO_TICK((ms)) : 1)
#define sleep_us(us)                sleep_hw_ticks(US_TO_HW_TICK((us)))

/* Polling on HW ticks. */
#define POLL_HW_US(expression, timeout, status, error_status)                       \
    {                                                                               \
        uint16_t __delta;                                                           \
        POLL_HW_US_D(expression, timeout, __delta, status, error_status);           \
    }
#define POLL_HW_US_Y(expression, timeout, status, error_status)                     \
    {                                                                               \
        uint16_t __delta;                                                           \
        POLL_HW_US_D_Y(expression, timeout, __delta, status, error_status);         \
    }
#define POLL_HW_US_D(expression, timeout, delta, status, error_status)              \
    {                                                                               \
        uint16_t __tick = (uint16_t)current_hardware_tick();                        \
        uint16_t __timeout_ticks = US_TO_HW_TICK(timeout);                          \
        do                                                                          \
        {                                                                           \
            delta = (uint16_t)((uint16_t)current_hardware_tick() - __tick);         \
            if (delta > __timeout_ticks)                                            \
                break;                                                              \
        } while (expression);                                                       \
        if (delta >= __timeout_ticks)                                               \
        {                                                                           \
            status = error_status;                                                  \
        }                                                                           \
    }
#define POLL_HW_US_D_Y(expression, timeout, delta, status, error_status)            \
    {                                                                               \
        uint16_t __tick = (uint16_t)current_hardware_tick();                        \
        uint16_t __timeout_ticks = US_TO_HW_TICK(timeout);                          \
        do                                                                          \
        {                                                                           \
            delta = (uint16_t)((uint16_t)current_hardware_tick() - __tick);         \
            if (delta > __timeout_ticks)                                            \
                break;                                                              \
            else                                                                    \
                task_yield();                                                       \
        } while (expression);                                                       \
        if (delta >= __timeout_ticks)                                               \
        {                                                                           \
            status = error_status;                                                  \
        }                                                                           \
    }

/* Polling on SW ticks. */
#define POLL_SW_MS(expression, timeout, status, error_status)                       \
    {                                                                               \
        uint16_t __delta;                                                           \
        POLL_SW_MS_D(expression, timeout, __delta, status, error_status);           \
    }
#define POLL_SW_MS_Y(expression, timeout, status, error_status)                     \
    {                                                                               \
        uint16_t __delta;                                                           \
        POLL_SW_MS_D_Y(expression, timeout, __delta, status, error_status);         \
    }
#define POLL_SW_MS_D(expression, timeout, delta, status, error_status)              \
    {                                                                               \
        uint16_t __tick = (uint16_t)current_system_tick();                          \
        uint16_t __timeout_ticks = (uint16_t)MS_TO_TICK(timeout);                   \
        do                                                                          \
        {                                                                           \
            delta = (uint16_t)((uint16_t)current_system_tick() - __tick);           \
            if (delta > __timeout_ticks)                                            \
                break;                                                              \
        } while (expression);                                                       \
        if (delta >= __timeout_ticks)                                               \
        {                                                                           \
            status = error_status;                                                  \
        }                                                                           \
    }
#define POLL_SW_MS_D_Y(expression, timeout, delta, status, error_status)            \
    {                                                                               \
        uint16_t __tick = (uint16_t)current_system_tick();                          \
        uint16_t __timeout_ticks = (uint16_t)MS_TO_TICK(timeout);                   \
        do                                                                          \
        {                                                                           \
            delta = (uint16_t)((uint16_t)current_system_tick() - __tick);           \
            if (delta > __timeout_ticks)                                            \
                break;                                                              \
            else                                                                    \
                task_yield();                                                       \
        } while (expression);                                                       \
        if (delta >= __timeout_ticks)                                               \
        {                                                                           \
            status = error_status;                                                  \
        }                                                                           \
    }

/* Kernel APIs. */
uint32_t current_system_tick(void);
uint8_t process_system_tick(void);

#endif /* CONFIG_SLEEP */

#endif /* _SLEEP_H_ */
