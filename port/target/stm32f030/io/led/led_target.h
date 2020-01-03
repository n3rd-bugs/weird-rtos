/*
 * led_target.h
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
#ifndef _LED_TARGET_H_
#define _LED_TARGET_H_
#include <kernel.h>

#ifdef CONFIG_LED
#include <led_stm32.h>

/* Hook-up LED OS stack. */
#define LED_TGT_INIT()              led_stm32_init()

#endif /* CONFIG_LED */
#endif /* _LED_TARGET_H_ */
