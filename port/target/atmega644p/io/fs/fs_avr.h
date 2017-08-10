/*
 * fs_avr.h
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _FS_AVR_H_
#define _FS_AVR_H_

#include <kernel.h>
#ifdef CONFIG_FS

#ifndef CMAKE_BUILD
/* Bit-bang SPI interface configuration for MMC card. */
#define MMC_ATMEGA644P_SPI_SS_BB            (3)
#define MMC_ATMEGA644P_SPI_PIN_SS_BB        (0x0)
#define MMC_ATMEGA644P_SPI_DDR_SS_BB        (0x1)
#define MMC_ATMEGA644P_SPI_PORT_SS_BB       (0x2)
#define MMC_ATMEGA644P_SPI_MOSI_BB          (5)
#define MMC_ATMEGA644P_SPI_PIN_MOSI_BB      (0x0)
#define MMC_ATMEGA644P_SPI_DDR_MOSI_BB      (0x1)
#define MMC_ATMEGA644P_SPI_PORT_MOSI_BB     (0x2)
#define MMC_ATMEGA644P_SPI_MISO_BB          (0)
#define MMC_ATMEGA644P_SPI_PIN_MISO_BB      (0x0)
#define MMC_ATMEGA644P_SPI_DDR_MISO_BB      (0x1)
#define MMC_ATMEGA644P_SPI_PORT_MISO_BB     (0x2)
#define MMC_ATMEGA644P_SPI_SCLK_BB          (2)
#define MMC_ATMEGA644P_SPI_PIN_SCLK_BB      (0x0)
#define MMC_ATMEGA644P_SPI_DDR_SCLK_BB      (0x1)
#define MMC_ATMEGA644P_SPI_PORT_SCLK_BB     (0x2)
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void fs_avr_init(void);

#endif /* CONFIG_FS */
#endif /* _FS_AVR_H_ */
