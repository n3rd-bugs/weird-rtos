/*
 * pcf8574.h
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
#ifndef _PCF8574_H_
#define _PCF8574_H_
#include <kernel.h>

#ifdef CONFIG_PCF8574
#ifndef CONFIG_I2C
#error "I2C is required for PCF8574."
#endif
#include <i2c.h>

/* This defines a PCF8574 device. */
typedef struct _pcf8574
{
    /* Associated I2C device. */
    I2C_DEVICE  i2c;

    /* Output mask. */
    uint8_t     out_mask;

    /* Data that is being output on the port. */
    uint8_t     out_data;

    /* Structure padding. */
    uint8_t     pad[2];

} PCF8574;

/* Function prototypes. */
int32_t pcf8574_init(PCF8574 *);
uint8_t pcf8574_read(PCF8574 *);
void pcf8574_write(PCF8574 *);

#endif /* CONFIG_PCF8574 */
#endif /* _PCF8574_H_ */
