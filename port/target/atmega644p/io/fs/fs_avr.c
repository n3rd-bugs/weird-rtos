/*
 * fs_avr.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef CONFIG_FS
#include <fs_avr.h>
#include <fs.h>

#if (defined(FS_FAT) && defined(CONFIG_MMC))
#include <mmc_spi.h>
#include <spi_bb_atmega644p.h>
#include <ffdiskio.h>
#include <ff.h>

/* MMC device definitions. */
static SPI_BB_AVR mmc_spi_bb;
static MMC_SPI mmc_spi;
static FATFS fat_fs;
#endif

/*
 * fs_avr_init
 * This function is responsible for initializing file systems for AVR.
 */
void fs_avr_init()
{
#if (defined(FS_FAT) && defined(CONFIG_MMC))
    char mount_point[4] = "0:\\";

#ifdef MMC_SPI_FS
    /* Register MMC device with file system */
    mmc_spi_fsregister(&mmc_spi, "\\mmc0");
#endif /* MMC_SPI_FS */

    /* Populate the SPI bit-bang interface. */
    mmc_spi_bb.pin_num_ss = 3;
    mmc_spi_bb.pin_num_mosi = 5;
    mmc_spi_bb.pin_num_miso = 0;
    mmc_spi_bb.pin_num_sclk = 2;
    mmc_spi_bb.pin_miso = mmc_spi_bb.pin_mosi = mmc_spi_bb.pin_ss = mmc_spi_bb.pin_sclk = 0x00;
    mmc_spi_bb.ddr_miso = mmc_spi_bb.ddr_mosi = mmc_spi_bb.ddr_ss = mmc_spi_bb.ddr_sclk = 0x01;
    mmc_spi_bb.port_miso = mmc_spi_bb.port_mosi = mmc_spi_bb.port_ss = mmc_spi_bb.port_sclk = 0x02;

    /* Hook-up SPI interface. */
    mmc_spi.spi.data = &mmc_spi_bb;
    mmc_spi.spi.init = &spi_bb_atmega644_init;
    mmc_spi.spi.slave_select = &spi_bb_atmega644_slave_select;
    mmc_spi.spi.slave_unselect = &spi_bb_atmega644_slave_unselect;
    mmc_spi.spi.msg = &spi_bb_atmega644_message;

    /* Register this device with FatFile system. */
    disk_register(&mmc_spi, &mmc_spi_init, &mmc_spi_read, &mmc_spi_write, MMC_SPI_SECTOR_SIZE, 0);

    /* Mount this drive later. */
    f_mount(&fat_fs, mount_point, 0);
#endif /* (defined(FS_FAT) && defined(CONFIG_MMC)) */

} /* fs_avr_init */
#endif /* CONFIG_FS */
