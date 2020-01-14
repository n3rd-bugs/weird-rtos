/*
 * dhtxx.h
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
#ifndef _DHTXX_H_
#define _DHTXX_H_
#include <kernel.h>

#ifdef CONFIG_DHT
/* DHT sensor type definitions. */
#define DHTXX_TYPE_22           (0)

/* DHTXX configuration. */
#define DHTXX_INIT_DELAY        (2000)
#define DHTXX_INIT_SIGNAL_DELAY (60)
#define DHTXX_SIGNAL_DELAY      (100)
#define DHTXX_DATA_LOW_DELAY    (100)
#define DHTXX_DATA_HIGH_DELAY   (100)
#define DHTXX_DATA_ZERO_DELAY   (60)
#define DHTXX_DATA_RETRY_DELAY  (100)
#define DHTXX_DATA_NUM_RETIES   (20)

/* Error code definitions. */
#define DHTXX_TIMEOUT           (-1900)
#define DHTXX_CSUM_ERROR        (-1901)
#define DHTXX_LINE_ERROR        (-1902)

/* Macro to convert raw temperature to 10's or centigrade */
#define DHTXX_TEMP(raw, temp)                                                       \
{                                                                                   \
    temp = raw & 0x7FFF;                                                            \
    if (raw & 0x8000)                                                               \
        temp = -temp;                                                               \
}

/* DHT sensor structure. */
typedef struct _dht_xx DHT_XX;
struct _dht_xx
{
    /* Platform hook-up for this device. */
    void (*pin_init) (DHT_XX *);
    void (*set_pin_mode) (DHT_XX *, uint8_t);
    uint8_t (*get_pin_state) (DHT_XX *);
    void (*set_pin_state) (DHT_XX *, uint8_t);
};

/* Function prototypes. */
void dhtxx_init(void);
int32_t dhtxx_register(DHT_XX *);
int32_t dhtxx_read(DHT_XX *, uint16_t *, uint16_t *, uint8_t);

/* Include DHTXX target definitions. */
#include <dhtxx_target.h>

#endif /* CONFIG_DHT */
#endif /* _DHTXX_H_ */
