/*
 * i2c_stm32f030.h
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _I2C_STM32F030_H_
#define _I2C_STM32F030_H_

#include <kernel.h>

#ifdef CONFIG_I2C
#include <i2c.h>
#ifdef STM_I2C_INT_MODE
#include <condition.h>
#endif /* STM_I2C_INT_MODE */

/* I2C configuration. */
#ifndef CMAKE_BUILD
//#define STM_I2C_BUSY_YIELD
#define STM_I2C_INT_TIMEOUT         50
//#define STM_I2C_INT_MODE
#endif /* CMAKE_BUILD */

/* I2C error flags. */
#define I2C_STM32_NACK              0x01
#define I2C_STM32_ERROR             0x02

/* Helper macros. */
#ifdef STM_I2C_BUSY_YIELD
#define I2C_STM_TIMED(expression)   {                                                                                                   \
                                        timeout = current_system_tick();                                                                \
                                        while ((expression) && ((current_system_tick() - timeout) < MS_TO_TICK(STM_I2C_INT_TIMEOUT)))   \
                                        {                                                                                               \
                                            task_yield();                                                                               \
                                        }                                                                                               \
                                        if ((current_system_tick() - timeout) >= MS_TO_TICK(STM_I2C_INT_TIMEOUT))                       \
                                        {                                                                                               \
                                            status = I2C_TIMEOUT;                                                                       \
                                        }                                                                                               \
                                    }
#else
#define I2C_STM_TIMED(expression)   {                                                                                                   \
                                        timeout = current_system_tick();                                                                \
                                        while ((expression) && ((current_system_tick() - timeout) < MS_TO_TICK(STM_I2C_INT_TIMEOUT)))   \
                                        {                                                                                               \
                                            ;                                                                                           \
                                        }                                                                                               \
                                        if ((current_system_tick() - timeout) >= MS_TO_TICK(STM_I2C_INT_TIMEOUT))                       \
                                        {                                                                                               \
                                            status = I2C_TIMEOUT;                                                                       \
                                        }                                                                                               \
                                    }
#endif /* STM_I2C_BUSY_YIELD */

/* STM32F030 I2C configuration structure. */
typedef struct _i2c_stm
{
    /* I2C device register. */
    I2C_TypeDef *i2c_reg;

    /* Timing for I2C device.  */
    uint32_t    timing;

#ifdef STM_I2C_INT_MODE
    /* I2C message being sent. */
    I2C_MSG     *msg;

    /* Condition to suspend on the interrupt. */
    CONDITION   condition;

    /* Suspend criteria. */
    SUSPEND     suspend;
#endif /* STM_I2C_INT_MODE */

#ifdef STM_I2C_INT_MODE
    /* Number of bytes transfered so far. */
    uint32_t    bytes_transfered;

    /* Number of bytes to be sent in this pass. */
    uint8_t     this_transfer;

    /* Flag to specify if we need to continue this request. */
    uint8_t     do_continue;

    /* Error flags. */
    uint8_t     flags;
#else
    /* Structure padding. */
    uint8_t     pad[3];
#endif /* STM_I2C_INT_MODE */

    /* I2C device number. */
    uint8_t     device_num;

} I2C_STM32;

/* Function prototypes. */
void i2c_stm32f030_init(I2C_DEVICE *);
int32_t i2c_stm32f030_message(I2C_DEVICE *, I2C_MSG *);
#ifdef STM_I2C_INT_MODE
ISR_FUN i2c1_event_interrupt(void);
#endif /* STM_I2C_INT_MODE */

#endif /* CONFIG_I2C */
#endif /* _I2C_STM32F030_H_ */
