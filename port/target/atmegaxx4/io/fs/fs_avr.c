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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>

#ifdef CONFIG_FS
#include <fs_avr.h>
#include <fs.h>

#if (defined(FS_FAT) && defined(CONFIG_MMC))
#include <mmc_spi.h>
#include <spi_bb_avr.h>
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
void fs_avr_init(void)
{
#if (defined(FS_FAT) && defined(CONFIG_MMC))
    char mount_point[4] = "0:\\";

#ifdef MMC_SPI_FS
    /* Register MMC device with file system */
    mmc_spi_fsregister(&mmc_spi, "\\mmc0");
#endif /* MMC_SPI_FS */

    /* Populate the SPI bit-bang interface. */
    mmc_spi_bb.pin_num_ss = MMC_AVR_SPI_SS_BB;
    mmc_spi_bb.pin_num_mosi = MMC_AVR_SPI_MOSI_BB;
    mmc_spi_bb.pin_num_miso = MMC_AVR_SPI_MISO_BB;
    mmc_spi_bb.pin_num_sclk = MMC_AVR_SPI_SCLK_BB;
    mmc_spi_bb.pin_ss = MMC_AVR_SPI_PIN_SS_BB;
    mmc_spi_bb.pin_mosi = MMC_AVR_SPI_PIN_MOSI_BB;
    mmc_spi_bb.pin_miso = MMC_AVR_SPI_PIN_MISO_BB;
    mmc_spi_bb.pin_sclk = MMC_AVR_SPI_PIN_SCLK_BB;
    mmc_spi_bb.ddr_ss = MMC_AVR_SPI_DDR_SS_BB;
    mmc_spi_bb.ddr_mosi = MMC_AVR_SPI_DDR_MOSI_BB;
    mmc_spi_bb.ddr_miso = MMC_AVR_SPI_DDR_MISO_BB;
    mmc_spi_bb.ddr_sclk = MMC_AVR_SPI_DDR_SCLK_BB;
    mmc_spi_bb.port_ss = MMC_AVR_SPI_PORT_SS_BB;
    mmc_spi_bb.port_mosi = MMC_AVR_SPI_PORT_MOSI_BB;
    mmc_spi_bb.port_miso = MMC_AVR_SPI_PORT_MISO_BB;
    mmc_spi_bb.port_sclk = MMC_AVR_SPI_PORT_SCLK_BB;

    /* Hook-up SPI interface. */
    mmc_spi.spi.cfg_flags = SPI_CFG_MASTER;
    mmc_spi.spi.data = &mmc_spi_bb;
    mmc_spi.spi.init = &spi_bb_avr_init;
    mmc_spi.spi.slave_select = &spi_bb_avr_slave_select;
    mmc_spi.spi.slave_unselect = &spi_bb_avr_slave_unselect;
    mmc_spi.spi.msg = &spi_bb_avr_message;

    /* Register this device with FatFile system. */
    disk_register(&mmc_spi, &mmc_spi_init, &mmc_spi_read, &mmc_spi_write, MMC_SPI_SECTOR_SIZE, 0);

    /* Mount this drive later. */
    f_mount(&fat_fs, mount_point, 0);
#endif /* (defined(FS_FAT) && defined(CONFIG_MMC)) */

} /* fs_avr_init */
#endif /* CONFIG_FS */
