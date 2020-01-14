/*
 * 1wire.h
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
#ifndef _1WIRE_H_
#define _1WIRE_H_
#include <kernel.h>

#ifdef CONFIG_1WIRE
/* Error code definitions. */
#define ONEWIRE_TIMEOUT             (-2000)
#define ONEWIRE_NO_DEVICE           (-2001)

/* 1-wire configuration. */
#define ONEWIRE_RESET_RETRY         (2)
#define ONEWIRE_INIT_HIGH_DELAY     (300)
#define ONEWIRE_RESET_DELAY         (480)
#define ONEWIRE_PRESENCE_DELAY      (70)
#define ONEWIRE_BIT_DELAY           (65)
#define ONEWIRE_BIT_HIGH_DELAY      (10)
#define ONEWIRE_BUS_TRIGGER         (3)

/* 1-wire definitions. */
#define ONEWIRE_CMD_CHOOSE_ROM      (0x55)
#define ONEWIRE_CMD_SKIP_ROM        (0xCC)
#define ONEWIRE_CMD_COND_SEARCH     (0xEC)
#define ONEWIRE_CMD_DEV_SEARCH      (0xF0)

/* 1wire configuration. */
typedef struct _one_wire ONE_WIRE;
struct _one_wire
{
    /* Platform hook-up for this device. */
    void (*pin_init) (ONE_WIRE *);
    void (*set_pin_mode) (ONE_WIRE *, uint8_t);
    uint8_t (*get_pin_state) (ONE_WIRE *);
    void (*set_pin_state) (ONE_WIRE *, uint8_t);

    /* Search state. */
    uint8_t     last_discrepancy;
    uint8_t     last_device;

    /* Structure padding. */
    uint8_t     pad[2];

};

/* Function prototypes. */
int32_t onewire_init(ONE_WIRE *);
int32_t onewire_reset(ONE_WIRE *);
int32_t onewire_select(ONE_WIRE *, const uint8_t *);
int32_t onewire_read_bit(ONE_WIRE *, uint8_t *);
int32_t onewire_read(ONE_WIRE *, uint8_t *, int32_t);
int32_t onewire_write_bit(ONE_WIRE *, uint8_t);
int32_t onewire_write(ONE_WIRE *, const uint8_t *, int32_t);
int32_t onewire_search(ONE_WIRE *, uint8_t, uint8_t *);
uint8_t onewire_crc(const uint8_t *, uint8_t);

#endif /* CONFIG_1WIRE */
#endif /* _1WIRE_H_ */
