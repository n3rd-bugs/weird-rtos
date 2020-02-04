/*
 * i2c_stm32f103.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _I2C_STM32F103_H_
#define _I2C_STM32F103_H_

#include <kernel.h>

#ifdef CONFIG_I2C
#include <i2c.h>
#include <i2c_stm32_config.h>
#ifdef STM_I2C_INT_MODE
#include <condition.h>
#endif /* STM_I2C_INT_MODE */

/* I2C error flags. */
#define I2C_STM32_NACK              0x1
#define I2C_STM32_ERROR             0x2

/* Helper macros. */
#ifdef STM_I2C_BUSY_YIELD
#define I2C_STM_TIMED(expression)   POLL_SW_MS_Y((expression), STM_I2C_INT_TIMEOUT, status, I2C_TIMEOUT)
#else
#define I2C_STM_TIMED(expression)   POLL_SW_MS((expression), STM_I2C_INT_TIMEOUT, status, I2C_TIMEOUT)
#endif /* STM_I2C_BUSY_YIELD */

/* STM32F103 I2C configuration structure. */
typedef struct _i2c_stm
{
    /* I2C device register. */
    I2C_TypeDef *i2c_reg;

    /* Speed for I2C device.  */
    uint32_t    speed;

#ifdef STM_I2C_INT_MODE
    /* I2C message being sent. */
    I2C_MSG     *msg;

    /* Condition to suspend on the interrupt. */
    CONDITION   condition;

    /* Suspend criteria. */
    SUSPEND     suspend;
#endif /* STM_I2C_INT_MODE */

    /* I2C device number. */
    uint8_t     device_num;

#ifdef STM_I2C_INT_MODE
    /* Number of bytes to be sent. */
    uint8_t     bytes_transfered;

    /* Error flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[1];
#else
    /* Structure padding. */
    uint8_t     pad[3];
#endif /* STM_I2C_INT_MODE */

} I2C_STM32;

/* Function prototypes. */
void i2c_stm32f103_init(I2C_DEVICE *);
int32_t i2c_stm32f103_message(I2C_DEVICE *, I2C_MSG *);
#ifdef STM_I2C_INT_MODE
ISR_FUN i2c1_event_interrupt(void);
ISR_FUN i2c1_error_interrupt(void);
#endif /* STM_I2C_INT_MODE */

#endif /* CONFIG_I2C */
#endif /* _I2C_STM32F103_H_ */
