/*
 * spi_avr.h
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
#ifndef _SPI_AVR_H_
#define _SPI_AVR_H_
#include <kernel.h>

#ifdef CONFIG_SPI
#include <spi.h>
#include <spi_avr_config.h>

/* SPCR register definitions. */
#define AVR_SPI_SPCR_SPIE_SHIFT     (7)
#define AVR_SPI_SPCR_SPE_SHIFT      (6)
#define AVR_SPI_SPCR_DORD_SHIFT     (5)
#define AVR_SPI_SPCR_MSTR_SHIFT     (4)
#define AVR_SPI_SPCR_CPOL_SHIFT     (3)
#define AVR_SPI_SPCR_CPHA_SHIFT     (2)
#define AVR_SPI_SPCR_SPR1_SHIFT     (1)
#define AVR_SPI_SPCR_SPR0_SHIFT     (0)

/* SPSR register definitions. */
#define AVR_SPI_SPSR_SPIF           (0x80)
#define AVR_SPI_SPSR_WCOL           (0x40)
#define AVR_SPI_SPSR_SPI2X          (0x01)

/* Function prototypes. */
void spi_avr_init(SPI_DEVICE *device);
void spi_avr_slave_select(SPI_DEVICE *);
void spi_avr_slave_unselect(SPI_DEVICE *);
int32_t spi_avr_message(SPI_DEVICE *, SPI_MSG *) SPEEDOPTIMIZATION;

#endif /* CONFIG_SPI */
#endif /* _SPI_AVR_H_ */
