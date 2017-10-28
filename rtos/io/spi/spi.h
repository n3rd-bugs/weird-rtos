/*
 * spi.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#ifndef _SPI_H_
#define _SPI_H_
#include <kernel.h>

#ifdef CONFIG_SPI

/* SPI error definitions. */
#define SPI_TIMEOUT             -1200

/* SPI configuration flags. */
#define SPI_CFG_1_WIRE          0x0001
#define SPI_CFG_RX_ONLY         0x0002
#define SPI_CFG_MODE_16BIT      0x0004
#define SPI_CFG_ENABLE_CRC      0x0008
#define SPI_CFG_LSB_FIRST       0x0010
#define SPI_CFG_MASTER          0x0020
#define SPI_CFG_CLK_IDLE_HIGH   0x0040
#define SPI_CFG_CLK_FIRST_DATA  0x0080
#define SPI_CFG_ENABLE_HARD_SS  0x0100

/* SPI message flags. */
#define SPI_MSG_READ            0x01
#define SPI_MSG_WRITE           0x02

/* SPI structure definitions. */
typedef struct _spi_device SPI_DEVICE;
typedef struct _spi_msg SPI_MSG;

/* SPI device hooks. */
typedef void (SPI_INIT)(SPI_DEVICE *);
typedef void (SPI_SLAVE_SELECT)(SPI_DEVICE *);
typedef void (SPI_SLAVE_UNSELECT)(SPI_DEVICE *);
typedef int32_t (SPI_MESSAGE)(SPI_DEVICE *, SPI_MSG *);

/* SPI device structure. */
struct _spi_device
{
    /* Target specific data. */
    void        *data;

    /* SPI device hooks. */
    SPI_INIT            *init;
    SPI_SLAVE_SELECT    *slave_select;
    SPI_SLAVE_UNSELECT  *slave_unselect;
    SPI_MESSAGE         *msg;

    /* SPI configuration flags. */
    uint32_t    cfg_flags;

    /* SPI baudrate configuration. */
    uint32_t    baudrate;

};

/* SPI message structure. */
struct _spi_msg
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
void spi_init(SPI_DEVICE *);
int32_t spi_message(SPI_DEVICE *, SPI_MSG *, uint32_t);

#endif /* CONFIG_SPI */

#endif /* _SPI_H_ */
