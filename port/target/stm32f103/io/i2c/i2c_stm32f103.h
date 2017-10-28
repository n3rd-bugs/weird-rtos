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
