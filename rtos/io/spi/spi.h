/*
 * spi.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _SPI_H_
#define _SPI_H_
#include <os.h>

#ifdef CONFIG_SPI

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

/* Week link SPI device structure. */
typedef struct _spi_device SPI_DEVICE;

/* Include SPI target configurations. */
#include <spi_target.h>

/* SPI device structure. */
typedef struct _spi_device
{
    /* Target specific data. */
    SPI_TGT_STRUCT  data;

    /* SPI configuration flags. */
    uint32_t        cfg_flags;

    /* SPI baudrate configuration. */
    uint32_t        baudrate;

} SPI_DEVICE;

/* Function prototypes. */
void spi_init(SPI_DEVICE *);
int32_t spi_write_read(SPI_DEVICE *, uint8_t *, int32_t, uint8_t *, int32_t);

#endif /* CONFIG_SPI */

#endif /* _SPI_H_ */
