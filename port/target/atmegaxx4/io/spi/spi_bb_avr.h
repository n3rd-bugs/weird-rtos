/*
 * spi_bb_avr.h
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
#ifndef _SPI_BB_AVR_H_
#define _SPI_BB_AVR_H_
#include <kernel.h>

#ifdef IO_SPI
#include <spi.h>

/* Bit-bang SPI configuration structure. */
typedef struct _spi_bb_avr
{
    /* Pin number configuration. */
    uint8_t pin_num_miso;
    uint8_t pin_num_mosi;
    uint8_t pin_num_ss;
    uint8_t pin_num_sclk;

    /* PORT registers. */
    uint8_t port_miso;
    uint8_t port_mosi;
    uint8_t port_ss;
    uint8_t port_sclk;

    /* PIN registers. */
    uint8_t pin_miso;
    uint8_t pin_mosi;
    uint8_t pin_ss;
    uint8_t pin_sclk;

    /* DDR registers. */
    uint8_t ddr_miso;
    uint8_t ddr_mosi;
    uint8_t ddr_ss;
    uint8_t ddr_sclk;
} SPI_BB_AVR;

/* Function prototypes. */
void spi_bb_avr_init(SPI_DEVICE *);
void spi_bb_avr_slave_select(SPI_DEVICE *);
void spi_bb_avr_slave_unselect(SPI_DEVICE *);
int32_t spi_bb_avr_message(SPI_DEVICE *, SPI_MSG *) SPEEDOPTIMIZATION;

#endif /* IO_SPI */
#endif /* _SPI_BB_AVR_H_ */
