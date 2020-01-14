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
#include <oled_avr_config.h>

/* Function prototypes. */
void oled_avr_init(void);

#endif /* CONFIG_OLED */
#endif /* _OLED_AVR_H_ */
