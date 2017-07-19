/*
 * i2c_bb_atmega644p.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _I2C_BB_ATMEGA644P_H_
#define _I2C_BB_ATMEGA644P_H_
#include <kernel.h>

#ifdef CONFIG_I2C
#include <i2c.h>

/* I2C configuration. */
#define ATMEGA644P_SLOW_I2C
#define ATMEGA644P_I2C_DELAY        (1)

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
void i2c_bb_atmega644_init(I2C_DEVICE *);
int32_t i2c_bb_atmega644_message(I2C_DEVICE *, I2C_MSG *) SPEEDOPTIMIZATION;

#endif /* CONFIG_I2C */
#endif /* _I2C_BB_ATMEGA644P_H_ */
