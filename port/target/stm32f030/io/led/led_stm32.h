/*
 * led_stm32.h
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
#ifndef _LED_STM32_H_
#define _LED_STM32_H_
#include <kernel.h>

#ifdef CONFIG_LED
#include <led_target.h>

#ifndef CMAKE_BUILD
/* LED configuration for STM32. */
#define LED_STM32_SPI_SPEED         (20000000)
#endif /* CMAKE_BUILD */

typedef struct _led_max7219
{

} LED_MAX7219;

/* Function prototypes. */
void led_stm32_init(void);

#endif /* CONFIG_LED */
#endif /* _LED_STM32_H_ */
