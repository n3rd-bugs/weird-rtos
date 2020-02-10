/*
 * dhtxx_target.h
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
#ifndef _DHTXX_TARGET_H_
#define _DHTXX_TARGET_H_
#include <kernel.h>

#ifdef GPIO_DHT
#include <dhtxx_stm32.h>

/* Export default DHT device. */
#define DHT_DEFAULT         (&sht_stm32.dht)

/* Hook-up DHT driver. */
#define DHT_TGT_INIT()      dhtxx_stm32_init()

#endif /* GPIO_DHT */
#endif /* _DHTXX_TARGET_H_ */
