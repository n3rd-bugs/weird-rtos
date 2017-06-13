/*
 * spi_atmega644p.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#ifndef _SPI_ATMEGA644P_H_
#define _SPI_ATMEGA644P_H_
#include <os.h>

#ifdef CONFIG_SPI
#include <spi.h>

/* SPCR register definitions. */
#define ATMEGA644P_SPI_SPCR_SPIE_SHIFT  (7)
#define ATMEGA644P_SPI_SPCR_SPE_SHIFT   (6)
#define ATMEGA644P_SPI_SPCR_DORD_SHIFT  (5)
#define ATMEGA644P_SPI_SPCR_MSTR_SHIFT  (4)
#define ATMEGA644P_SPI_SPCR_CPOL_SHIFT  (3)
#define ATMEGA644P_SPI_SPCR_CPHA_SHIFT  (2)
#define ATMEGA644P_SPI_SPCR_SPR1_SHIFT  (1)
#define ATMEGA644P_SPI_SPCR_SPR0_SHIFT  (0)

/* SPSR register definitions. */
#define ATMEGA644P_SPI_SPSR_SPIF    (0x80)
#define ATMEGA644P_SPI_SPSR_WCOL    (0x40)
#define ATMEGA644P_SPI_SPSR_SPI2X   (0x01)

/* Maximum timeout to wait for SPI message to process. */
#define ATMEGA644P_SPI_TIMEOUT      (100)

/* Function prototypes. */
void spi_atmega644_init();
void spi_atmega644_slave_select(SPI_DEVICE *);
void spi_atmega644_slave_unselect(SPI_DEVICE *);
int32_t spi_atmega644_message(SPI_DEVICE *, SPI_MSG *) SPEEDOPTIMIZATION;

#endif /* CONFIG_SPI */
#endif /* _SPI_ATMEGA644P_H_ */
