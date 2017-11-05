/*
 * fs_stm32f103.c
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
#include <kernel.h>

#ifdef CONFIG_FS
#include <fs_stm32f103.h>
#include <fs.h>

#if (defined(FS_FAT) && defined(CONFIG_MMC))
#include <spi_stm32f103.h>
#include <mmc_spi.h>
#include <ffdiskio.h>
#include <ff.h>

/* MMC device definitions. */
static STM32F103_SPI mmc_spi_stm32;
static MMC_SPI mmc_spi;
static FATFS fat_fs;
#endif

/*
 * fs_stm32_init
 * This function is responsible for initializing file systems for STM32.
 */
void fs_stm32_init(void)
{
#if (defined(FS_FAT) && defined(CONFIG_MMC))
    char mount_point[4] = "0:\\";

#ifdef MMC_SPI_FS
    /* Register MMC device with file system */
    mmc_spi_fsregister(&mmc_spi, "\\mmc0");
#endif /* MMC_SPI_FS */

    /* Initialize SPI device data. */
    mmc_spi_stm32.device_num = 2;

    /* Hook-up SPI interface. */
    mmc_spi.spi.baudrate = 21000000;
    mmc_spi.spi.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_IDLE_HIGH);
    mmc_spi.spi.data = &mmc_spi_stm32;
    mmc_spi.spi.init = &spi_stm32f103_init;
    mmc_spi.spi.slave_select = &spi_stm32f103_slave_select;
    mmc_spi.spi.slave_unselect = &spi_stm32f103_slave_unselect;
    mmc_spi.spi.msg = &spi_stm32f103_message;

    /* Register this device with FatFile system. */
    disk_register(&mmc_spi, &mmc_spi_init, &mmc_spi_read, &mmc_spi_write, MMC_SPI_SECTOR_SIZE, 0);

    /* Mount this drive later. */
    f_mount(&fat_fs, mount_point, 0);
#endif /* (defined(FS_FAT) && defined(CONFIG_MMC)) */

} /* fs_avr_init */
#endif /* CONFIG_FS */
