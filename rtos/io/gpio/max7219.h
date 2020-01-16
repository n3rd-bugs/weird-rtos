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

#ifdef CONFIG_MAX7219
#ifndef CONFIG_SPI
#error "SPI is required for MAX7219."
#endif /* CONFIG_SPI */
#include <spi.h>

/* This defines a MAX7219 device. */
typedef struct _max7219
{
    /* Associated SPI device. */
    SPI_DEVICE  spi;

} MAX7219;

/* This defines LED segment display over MAX7219. */
typedef struct _led_max7219
{
    /* Associated MAX7219 device. */
    MAX7219     max;

} LED_MAX7219;

/* Function prototypes. */
void max7219_init(void);
int32_t max7219_register(MAX7219 *);
int32_t led_max7219_register(LED_MAX7219 *);

#endif /* CONFIG_MAX7219 */
#endif /* _MAX7219_H_ */
