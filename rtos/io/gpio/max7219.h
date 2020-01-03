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

/* This defines a PCF8574 device. */
typedef struct _max7219
{
    /* Associated SPI device. */
    SPI_DEVICE  spi;

} MAX7219;

#endif /* CONFIG_MAX7219 */
#endif /* _MAX7219_H_ */
