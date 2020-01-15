/*
 * ds182x.h
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
#ifndef _DS182X_H_
#define _DS182X_H_
#include <kernel.h>

#ifdef CONFIG_DS182X
#include <1wire.h>

/* Error code definitions. */
#define DS182X_CONV_TIMEOUT     (-2100)

/* Supported model IDs. */
#define DS182X_DS18S20          (0x10)
#define DS182X_DS18B20          (0x28)
#define DS182X_DS1822           (0x22)
#define DS182X_DS1825           (0x3B)

/* DS182x command definitions. */
#define DS182X_STARTCONV        (0x44)
#define DS182X_READSCRATCH      (0xBE)

/* DS182x scratch pad byte definitions. */
#define DS182X_SC_TEMP_LSB      (0)
#define DS182X_SC_TEMP_MSB      (1)
#define DS182X_SC_HIGH_ALARM    (2)
#define DS182X_SC_LOW_ALARM     (3)
#define DS182X_SC_CONFIG        (4)
#define DS182X_SC_INT_BYTE      (5)
#define DS182X_SC_COUNT_REMAIN  (6)
#define DS182X_SC_COUNT_PER_C   (7)
#define DS182X_SC_CRC           (8)
#define DS182X_SC_SIZE          (9)

/* DS182x configurations. */
#define DS182X_CONV_MAX_TIME    1200

/* DS182X sensor structure. */
typedef struct _ds182x
{
    /* 1-wire bus structure. */
    ONE_WIRE    onewire;

    /* Search state for the DS182x bus. */
    uint8_t rom_addr[8];

} DS182X;

/* Function prototypes. */
void ds182x_init(void);
int32_t ds182x_register(DS182X *);
int32_t ds182x_get_first(DS182X *, uint16_t *);
int32_t ds182x_get_next(DS182X *, uint16_t *);

/* Include DS182x target definitions. */
#include <ds182x_target.h>

#endif /* CONFIG_DS182X */
#endif /* _DS182X_H_ */
