/*
 * ds182x_stm32.h
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
#ifndef _DS182X_STM32_H_
#define _DS182X_STM32_H_
#include <kernel.h>

#ifdef GPIO_DS182X
#include <ds182x.h>
#include <ds182x_stm32_config.h>

/* DS182X bus structure. */
typedef struct _ds182x_stm32
{
    /* DS182X bus. */
    DS182X          bus;

    /* GPIO configuration. */
    GPIO_TypeDef    *port;
    uint8_t         pin;

    /* Structure padding. */
    uint8_t         pad[3];
} DS182X_STM32;

/* Exported variables. */
extern DS182X_STM32 ds182x_stm32;

/* Function prototypes. */
void ds182x_stm32_init();

#endif /* GPIO_DS182X */
#endif /* _DS182X_STM32_H_ */
