/*
 * i2c.h
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
#ifndef _I2C_H_
#define _I2C_H_
#include <kernel.h>

#ifdef CONFIG_I2C

/* I2C error definitions. */
#define I2C_MSG_NACKED          -1600
#define I2C_TIMEOUT             -1601
#define I2C_IO_ERROR            -1602

/* I2C configuration flags. */

/* I2C message flags. */
#define I2C_MSG_READ            0x1
#define I2C_MSG_WRITE           0x2

/* I2C structure definitions. */
typedef struct _i2c_device I2C_DEVICE;
typedef struct _i2c_msg I2C_MSG;

/* I2C device hooks. */
typedef void (I2C_INIT)(I2C_DEVICE *);
typedef void (I2C_SLAVE_SELECT)(I2C_DEVICE *);
typedef void (I2C_SLAVE_UNSELECT)(I2C_DEVICE *);
typedef int32_t (I2C_MESSAGE)(I2C_DEVICE *, I2C_MSG *);

/* I2C device structure. */
struct _i2c_device
{
    /* Target specific data. */
    void        *data;

    /* I2C device hooks. */
    I2C_INIT    *init;
    I2C_MESSAGE *msg;

    /* I2C device address */
    uint8_t     address;

    /* Structure padding. */
    uint8_t     pad[3];
};

/* I2C message structure. */
struct _i2c_msg
{
    /* Buffer for read or write operation. */
    uint8_t     *buffer;

    /* Buffer length. */
    int32_t     length;

    /* Message flags. */
    uint8_t     flags;

    /* Structure padding. */
    uint8_t     pad[3];
};

/* Function prototypes. */
void i2c_init(I2C_DEVICE *);
int32_t i2c_message(I2C_DEVICE *, I2C_MSG *, uint32_t);

#endif /* CONFIG_I2C */
#endif /* _I2C_H_ */
