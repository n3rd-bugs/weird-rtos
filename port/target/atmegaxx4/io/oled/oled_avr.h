/*
 * oled_avr.h
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
#ifndef _OLED_AVR_H_
#define _OLED_AVR_H_
#include <kernel.h>

#ifdef CONFIG_OLED
#include <oled_target.h>

#ifndef CMAKE_BUILD
/* OLED configuration for AVR. */
#define OLED_AVR_I2C_ADDRESS        (0x3C)
#define OLED_AVR_PIN_SCL            (0x3)
#define OLED_AVR_PIN_SDA            (0x3)
#define OLED_AVR_DDR_SCL            (0x4)
#define OLED_AVR_DDR_SDA            (0x4)
#define OLED_AVR_PORT_SCL           (0x5)
#define OLED_AVR_PORT_SDA           (0x5)
#define OLED_AVR_PIN_NUM_SCL        (0)
#define OLED_AVR_PIN_NUM_SDA        (1)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void oled_avr_init(void);

#endif /* CONFIG_OLED */
#endif /* _OLED_AVR_H_ */
