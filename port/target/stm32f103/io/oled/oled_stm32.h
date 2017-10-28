/*
 * oled_stm32.h
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
