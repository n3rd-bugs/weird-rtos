/*
 * i2c_stm32f103.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _I2C_STM32F103_H_
#define _I2C_STM32F103_H_

#include <kernel.h>

#ifdef CONFIG_I2C
#include <i2c.h>

/* I2C configuration. */
#ifndef CMAKE_BUILD
#define STM_I2C_WAIT        (250)
#endif /* CMAKE_BUILD */

/* Helper macros. */
#define I2C_STM_IS_EVENT(i2c, event)    ((((uint32_t)(i2c)->i2c_reg->SR1 | ((uint32_t)(i2c)->i2c_reg->SR2 << 16)) & event) == event)

/* STM32F103 I2C configuration structure. */
typedef struct _i2c_stm
{
    /* I2C device register. */
    I2C_TypeDef *i2c_reg;

    /* Speed for I2C device.  */
    uint32_t    speed;

    /* I2C device number. */
    uint8_t     device_num;

    /* Structure padding. */
    uint8_t     pad[3];

} I2C_STM32;

/* Function prototypes. */
void i2c_stm32f103_init(I2C_DEVICE *);
int32_t i2c_stm32f103_message(I2C_DEVICE *, I2C_MSG *);

#endif /* CONFIG_I2C */
#endif /* _I2C_STM32F103_H_ */
