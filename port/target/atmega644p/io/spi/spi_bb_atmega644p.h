/*
 * spi_bb_atmega644p.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _SPI_BB_ATMEGA644P_H_
#define _SPI_BB_ATMEGA644P_H_
#include <kernel.h>

#ifdef CONFIG_SPI
#include <spi.h>

/* Maximum timeout to wait for SPI message to process. */
#define ATMEGA644P_SPI_TIMEOUT      (100)

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
void spi_bb_atmega644_init(SPI_DEVICE *);
void spi_bb_atmega644_slave_select(SPI_DEVICE *);
void spi_bb_atmega644_slave_unselect(SPI_DEVICE *);
int32_t spi_bb_atmega644_message(SPI_DEVICE *, SPI_MSG *) SPEEDOPTIMIZATION;

#endif /* CONFIG_SPI */
#endif /* _SPI_BB_ATMEGA644P_H_ */
