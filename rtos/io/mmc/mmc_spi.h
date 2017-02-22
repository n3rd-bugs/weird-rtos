/*
 * mmc_spi.h
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
#ifndef _MMC_SPI_H_
#define _MMC_SPI_H_
#include <os.h>

#ifdef CONFIG_MMC
#ifndef CONFIG_SPI
#error "SPI is required for MMC SPI."
#endif
#include <spi.h>

/* MMC error definitions. */
#define MMC_SPI_CMD_ERROR       -1500
#define MMC_SPI_READ_ERROR      -1500
#define MMC_SPI_WRITE_ERROR     -1502

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

/* MMC SPI structure. */
typedef struct _mmc_spi
{
    /* SPI device configuration structure. */
    SPI_DEVICE  spi;

    /* Card flags. */
    uint8_t     flags;

} MMC_SPI;

/* Function prototypes. */
int32_t mmc_spi_init(MMC_SPI *);
int32_t mmc_spi_read(MMC_SPI *, uint32_t, uint64_t *, uint8_t *, int32_t);
int32_t mmc_spi_write(MMC_SPI *, uint32_t, uint64_t *, uint8_t *, int32_t);
int32_t mmc_spi_get_num_sectors(MMC_SPI *, uint64_t *);
int32_t mmc_spi_get_sectors_per_block(MMC_SPI *, uint64_t *);

#endif /* CONFIG_MMC */
#endif /* _MMC_SPI_H_ */
