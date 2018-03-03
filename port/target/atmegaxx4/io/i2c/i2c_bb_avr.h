/*
 * i2c_bb_avr.h
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
#ifndef _I2C_BB_AVR_H_
#define _I2C_BB_AVR_H_
#include <kernel.h>

#ifdef CONFIG_I2C
#include <i2c.h>

/* I2C configuration. */
#ifdef CMAKE_BUILD
#include <i2c_bb_avr_config.h>
#else
#define AVR_SLOW_I2C
#define AVR_I2C_DELAY        (1)
#endif /* CMAKE_BUILD */

/* Bit-bang I2C configuration structure. */
typedef struct _i2c_bb_avr
{
    /* Pin number configuration. */
    uint8_t pin_num_scl;
    uint8_t pin_num_sda;

    /* PORT registers. */
    uint8_t port_scl;
    uint8_t port_sda;

    /* PIN registers. */
    uint8_t pin_scl;
    uint8_t pin_sda;

    /* DDR registers. */
    uint8_t ddr_scl;
    uint8_t ddr_sda;

} I2C_BB_AVR;

/* Function prototypes. */
void i2c_bb_avr_init(I2C_DEVICE *);
int32_t i2c_bb_avr_message(I2C_DEVICE *, I2C_MSG *) SPEEDOPTIMIZATION;

#endif /* CONFIG_I2C */
#endif /* _I2C_BB_AVR_H_ */
