/*
 * fs_avr.h
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
#ifndef _FS_AVR_H_
#define _FS_AVR_H_
#include <kernel.h>

#ifdef CONFIG_FS

#ifndef CMAKE_BUILD
#if (defined(FS_FAT) && defined(CONFIG_MMC))
/* Bit-bang SPI interface configuration for MMC card. */
#define MMC_AVR_SPI_SS_BB               (3)
#define MMC_AVR_SPI_PIN_SS_BB           (0x0)
#define MMC_AVR_SPI_DDR_SS_BB           (0x1)
#define MMC_AVR_SPI_PORT_SS_BB          (0x2)
#define MMC_AVR_SPI_MOSI_BB             (5)
#define MMC_AVR_SPI_PIN_MOSI_BB         (0x0)
#define MMC_AVR_SPI_DDR_MOSI_BB         (0x1)
#define MMC_AVR_SPI_PORT_MOSI_BB        (0x2)
#define MMC_AVR_SPI_MISO_BB             (0)
#define MMC_AVR_SPI_PIN_MISO_BB         (0x0)
#define MMC_AVR_SPI_DDR_MISO_BB         (0x1)
#define MMC_AVR_SPI_PORT_MISO_BB        (0x2)
#define MMC_AVR_SPI_SCLK_BB             (2)
#define MMC_AVR_SPI_PIN_SCLK_BB         (0x0)
#define MMC_AVR_SPI_DDR_SCLK_BB         (0x1)
#define MMC_AVR_SPI_PORT_SCLK_BB        (0x2)
#endif /* (defined(FS_FAT) && defined(CONFIG_MMC)) */
#endif /* CMAKE_BUILD */

/* Function prototypes. */
void fs_avr_init(void);

#endif /* CONFIG_FS */
#endif /* _FS_AVR_H_ */
