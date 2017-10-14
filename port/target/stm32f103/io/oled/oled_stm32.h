/*
 * oled_stm32.h
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
#ifndef _OLED_STM32_H_
#define _OLED_STM32_H_
#include <kernel.h>

#ifdef CONFIG_OLED
#include <oled_target.h>

#ifndef CMAKE_BUILD
/* OLED configuration for STM32. */
#define OLED_STM32_I2C_ADDRESS      (0x3C)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void oled_stm32_init(void);

#endif /* CONFIG_OLED */
#endif /* _OLED_STM32_H_ */
