/*
 * oled_stm32.h
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
#ifndef _OLED_STM32_H_
#define _OLED_STM32_H_
#include <kernel.h>

#ifdef IO_OLED
#include <oled_target.h>
#include <oled_stm32_config.h>

/* Function prototypes. */
void oled_stm32_init(void);

#endif /* IO_OLED */
#endif /* _OLED_STM32_H_ */
