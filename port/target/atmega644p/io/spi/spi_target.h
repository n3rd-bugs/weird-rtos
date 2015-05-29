/*
 * spi_target.h
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

#ifndef _SPI_TARGET_H_
#define _SPI_TARGET_H_

#include <os.h>

#ifdef CONFIG_SPI
#include <spi_atmega644p.h>

/* Hook-up SPI OS stack. */
typedef struct _atmega644p_spi  SPI_TGT_STRUCT;
#define SPI_TGT_INIT            spi_atmega644_init
#define SPI_TGT_SS              spi_atmega644_slave_select
#define SPI_TGT_SUS             spi_atmega644_slave_unselect
#define SPI_TGT_MSG             spi_atmega644_message

#endif /* CONFIG_SPI */

#endif /* _SPI_TARGET_H_ */
