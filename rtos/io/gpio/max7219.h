/*
 * max7219.h
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
#ifndef _MAX7219_H_
#define _MAX7219_H_
#include <kernel.h>

#ifdef GPIO_MAX7219
#ifndef IO_SPI
#error "SPI is required for MAX7219."
#endif /* IO_SPI */
#include <spi.h>
#include <gfx.h>
#include <max7219_config.h>

/* Error code definitions. */
#define MAX7219_NOT_SUPPORTED   -2200
#define MAX7219_OUT_OF_RANGE    -2201

/* MAX7219 address definitions. */
#define MAX7219_ADDR_NOOP           0x0
#define MAX7219_ADDR_DIGIT_0        0x1
#define MAX7219_ADDR_DIGIT_1        0x2
#define MAX7219_ADDR_DIGIT_2        0x3
#define MAX7219_ADDR_DIGIT_3        0x4
#define MAX7219_ADDR_DIGIT_4        0x5
#define MAX7219_ADDR_DIGIT_5        0x6
#define MAX7219_ADDR_DIGIT_6        0x7
#define MAX7219_ADDR_DIGIT_7        0x8
#define MAX7219_ADDR_DECODE_MODE    0x9
#define MAX7219_ADDR_INTENSITY      0xA
#define MAX7219_ADDR_SCAN_LIMIT     0xB
#define MAX7219_ADDR_SHUTDOWN       0xC
#define MAX7219_ADDR_DISPLAY_TEST   0xF

/* This defines a MAX7219 device. */
typedef struct _max7219
{
    /* Associated SPI device. */
    SPI_DEVICE  spi;

} MAX7219;

/* This defines LED segment display over MAX7219. */
typedef struct _led_max7219
{
    /* Graphics data for this driver. */
    GFX         gfx;

    /* Associated MAX7219 device. */
    MAX7219     max;

} LED_MAX7219;

/* Function prototypes. */
void max7219_init(void);
int32_t max7219_register(MAX7219 *);
int32_t max7219_set_power(MAX7219 *, uint8_t);
int32_t max7219_set_decode(MAX7219 *, uint8_t);
int32_t led_max7219_register(LED_MAX7219 *);

#endif /* GPIO_MAX7219 */
#endif /* _MAX7219_H_ */
