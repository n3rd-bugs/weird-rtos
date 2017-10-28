/*
 * mmc_spi.h
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
#ifndef _MMC_SPI_H_
#define _MMC_SPI_H_
#include <kernel.h>

#ifdef CONFIG_MMC
#ifndef CONFIG_SPI
#error "SPI is required for MMC SPI."
#endif
#include <spi.h>

/* Enable file system interface of MMC. */
#ifndef CMAKE_BUILD
#define MMC_SPI_FS
#endif /* CMAKE_BUILD */

#ifdef MMC_SPI_FS
#include <fs.h>
#ifdef CONFIG_SEMAPHORE
#include <semaphore.h>
#else
#error "Semaphores are required for FS interface."
#endif
#endif

/* MMC error definitions. */
#define MMC_SPI_CMD_ERROR       -1500
#define MMC_SPI_READ_ERROR      -1501
#define MMC_SPI_WRITE_ERROR     -1502
#define MMC_SPI_SELECT_ERROR    -1503
#define MMC_SPI_RESUME_ERROR    -1504
#define MMC_SPI_INVALID_PARAM   -1505

/* MMC read/write offset definitions. */
#define MMC_SPI_START_OFFSET    (uint64_t)(-1)
#define MMC_SPI_UNKNOWN_NBYTES  (uint64_t)(-1)

/* MMC SPI command definitions. */
#define MMC_SPI_CMD0            (0x00)
#define MMC_SPI_CMD1            (0x01)
#define MMC_SPI_CMD8            (0x08)
#define MMC_SPI_CMD9            (0x09)
#define MMC_SPI_CMD12           (0x0C)
#define MMC_SPI_CMD16           (0x10)
#define MMC_SPI_CMD18           (0x12)
#define MMC_SPI_CMD25           (0x19)
#define MMC_SPI_CMD55           (0x37)
#define MMC_SPI_CMD58           (0x3A)
#define MMC_SPI_ACMD41          (0x29)
#define MMC_SPI_ACMD51          (0x33)

/* CMD58 response bits. */
#define MMC_SPI_CMD58_BM        (0x40)

/* MMC SPI definitions. */
#define MMC_SPI_LINE_RETRIES    (100)
#define MMC_SPI_IDLE_RETRIES    (2)
#define MMC_SPI_RESUME_RETRIES  (10)
#define MMC_SPI_RX_RETRIES      (15)
#define MMC_SPI_TX_RETRIES      (150)
#define MMC_SPI_CMD_RETRIES     (15)
#define MMC_SPI_RESUME_DELAY    (50)
#define MMC_SPI_SELECT_DELAY    (10)
#define MMC_SPI_RX_DELAY        (10)
#define MMC_SPI_TX_DELAY        (10)
#define MMC_SPI_CMD_LEN         (0x06)
#define MMC_SPI_CSD_LEN         (0x10)
#define MMC_SPI_STATUS_LEN      (0x40)
#define MMC_SPI_CMD8_ARG        (0x000001AA)
#define MMC_SPI_ACMD41_ARG      (0x40000000)
#define MMC_SPI_R1_IDLE         (0x01)
#define MMC_SPI_R1_COMP         (0x80)
#define MMC_SPI_DATA_TX_TKN     (0xFC)
#define MMC_SPI_DATA_ST_TKN     (0xFD)
#define MMC_SPI_DATA_RX_TKN     (0xFE)
#define MMC_SPI_SECTOR_SIZE     (0x200)

/* MMC command flags. */
#define MMC_SPI_CMD_SEL         (0x01)
#define MMC_SPI_CMD_UNSEL       (0x02)

/* MMC card type definitions. */
#define MMC_SPI_CARD_MMC        (0x01)
#define MMC_SPI_CARD_SD1        (0x02)
#define MMC_SPI_CARD_SD2        (0x04)
#define MMC_SPI_CARD_SD         ((MMC_SPI_CARD_SD1) | (MMC_SPI_CARD_SD2))
#define MMC_SPI_CARD_BLOCK      (0x08)
#define MMC_SPI_INIT_COMPLETE   (0x10)
#define MMC_SPI_OPEN_READ       (0x20)
#define MMC_SPI_OPEN_WRITE      (0x40)

/* MMC SPI structure. */
typedef struct _mmc_spi
{
#ifdef MMC_SPI_FS
    /* File system hook for this device. */
    FS          fs;

    /* Semaphore to protect access to this MMC device. */
    SEMAPHORE   lock;

    /* Structure padding. */
    uint32_t    pad32[1];

    /* Current offset in the flash. */
    uint64_t    offset;

    /* Number of bytes we need to read or write. */
    uint64_t    num_bytes;
#endif

    /* SPI device configuration structure. */
    SPI_DEVICE  spi;

    /* Card flags. */
    uint16_t    flags;

    /* Structure padding. */
    uint8_t     pad8[2];
} MMC_SPI;

/* Function prototypes. */
#ifdef MMC_SPI_FS
void mmc_spi_fsregister(void *, const char *);
#endif
int32_t mmc_spi_init(void *);
int32_t mmc_spi_read(void *, uint32_t, uint64_t *, uint8_t *, int32_t);
int32_t mmc_spi_write(void *, uint32_t, uint64_t *, uint8_t *, int32_t);
int32_t mmc_spi_get_num_sectors(MMC_SPI *, uint64_t *);
int32_t mmc_spi_get_sectors_per_block(MMC_SPI *, uint64_t *);

#endif /* CONFIG_MMC */
#endif /* _MMC_SPI_H_ */
