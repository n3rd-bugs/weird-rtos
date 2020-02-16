/*
 * led_max7219_stm32.h
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
#ifndef _LED_MAX7219_STM32_H_
#define _LED_MAX7219_STM32_H_
#include <kernel.h>

#ifdef GPIO_MAX7219
#include <max7219.h>

/* LED MAX7219 structure for STM32. */
typedef struct _led_max7219_stm32
{
    /* MAX7219 instance. */
    LED_MAX7219     led_max;

} LED_MAX7219_STM32;

/* Function prototypes. */
void led_max7219_stm32_init(void);

#endif /* GPIO_MAX7219 */
#endif /* _LED_MAX7219_STM32_H_ */
