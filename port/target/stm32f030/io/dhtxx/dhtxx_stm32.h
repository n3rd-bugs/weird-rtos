/*
 * dhtxx_stm32.h
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
#ifndef _DHTXX_STM32_H_
#define _DHTXX_STM32_H_
#include <kernel.h>

#ifdef GPIO_DHT
#include <dhtxx.h>
#include <dhtxx_stm32_config.h>

/* DHT sensor structure. */
typedef struct _dht_xx_stm32
{
    /* DHT device. */
    DHT_XX          dht;

    /* GPIO configuration. */
    GPIO_TypeDef    *port;
    uint8_t         pin;

    /* Structure padding. */
    uint8_t         pad[3];
} DHT_XX_STM32;

/* Exported variables. */
extern DHT_XX_STM32 sht_stm32;

/* Function prototypes. */
void dhtxx_stm32_init();

#endif /* GPIO_DHT */
#endif /* _DHTXX_STM32_H_ */
