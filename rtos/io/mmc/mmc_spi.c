/*
 * mmc_spi.c
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
#ifdef CONFIG_MMC
#include <mmc_spi.h>

/* Internal API definitions. */
static int32_t mmc_spi_get_csd(MMC_SPI *, uint8_t *);
static int32_t mmc_spi_rx_data(MMC_SPI *, uint8_t *, int32_t);
static int32_t mmc_spi_cmd(MMC_SPI *, uint8_t, uint8_t, uint32_t, uint8_t *, int32_t);
static int32_t mmc_slave_select(MMC_SPI *);
static int32_t mmc_slave_unselect(MMC_SPI *);

/*
 * mmc_spi_init
 * @mmc: MMC SPI device needed to be initialized.
 * @return: Success will be returned if MMC device was successfully initialized.
 * This function will initialize a MMC/SD device over SPI bus.
 */
int32_t mmc_spi_init(MMC_SPI *mmc)
{
    int32_t status;
    SPI_MSG msg;
    uint8_t resp[5];

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Initialize the SPI bus. */
    mmc->spi.init(&mmc->spi);

    /* Initialize a dummy message. */
    msg.buffer = resp;
    msg.length = 5;
    msg.flags = (SPI_MSG_READ);

    /* Insert 80 (5 * 2 * 8) dummy clocks on SPI bus. */
    mmc->spi.msg(&mmc->spi, &msg);
    mmc->spi.msg(&mmc->spi, &msg);

    /* Send CMD0 on MMC. */
    status = mmc_spi_cmd(mmc, MMC_SPI_CMD0, MMC_SPI_CMD_SEL, 0x0, NULL, 0);

    if (status == SUCCESS)
    {
        /* Send CMD8 on MMC. */
        status = mmc_spi_cmd(mmc, MMC_SPI_CMD8, 0, MMC_SPI_CMD8_ARG, resp, 5);

        if (status == SUCCESS)
        {
            /* Check if we are SDv2. */
            if ((resp[3] == 0x01) && (resp[4] == 0xAA))
            {
                /* Let get card out of idle. */
                while (status == SUCCESS)
                {
                    /* Send CMD55. */
                    status = mmc_spi_cmd(mmc, MMC_SPI_CMD55, 0, 0, NULL, 0);

                    if (status == SUCCESS)
                    {
                        /* Send ACMD41 on MMC. */
                        status = mmc_spi_cmd(mmc, MMC_SPI_ACMD41, 0, MMC_SPI_ACMD41_ARG, resp, 1);
                    }

                    /* If card is now out of idle. */
                    if ((resp[0] & MMC_SPI_R1_IDLE) == 0)
                    {
                        break;
                    }
                }

                if (status == SUCCESS)
                {
                    /* Send CMD58 on MMC. */
                    status = mmc_spi_cmd(mmc, MMC_SPI_CMD58, 0, 0x0, resp, 5);
                }

                if (status == SUCCESS)
                {
                    /* This is SDv2 card. */
                    mmc->flags = MMC_SPI_CARD_SD2;

                    /* If card supports block mode. */
                    if (resp[0] & MMC_SPI_CMD58_BM)
                    {
                        /* Set the block mode flag. */
                        mmc->flags |= MMC_SPI_CARD_BLOCK;
                    }

                    SYS_LOG_FUNTION_HEXDUMP(MMC, SYS_LOG_INFO, "OCR: 0x", &resp[1], 4);
                }
            }

            /* We must be SDv1 or MMCv3. */
            else
            {
                /* Send ACMD41 on MMC. */
                resp[0] = 0xFF;
                mmc_spi_cmd(mmc, MMC_SPI_CMD55, 0, 0, NULL, 0);
                mmc_spi_cmd(mmc, MMC_SPI_ACMD41, 0, MMC_SPI_ACMD41_ARG, resp, 1);

                /* If we got a valid response. */
                if (resp[0] <= 1)
                {
                    /* Let get card out of idle. */
                    while (status == SUCCESS)
                    {
                        /* Send CMD55. */
                        status = mmc_spi_cmd(mmc, MMC_SPI_CMD55, 0, 0, resp, 1);

                        if (status == SUCCESS)
                        {
                            /* Send ACMD41 on MMC. */
                            status = mmc_spi_cmd(mmc, MMC_SPI_ACMD41, 0, MMC_SPI_ACMD41_ARG, resp, 1);
                        }

                        /* If card is now out of idle. */
                        if ((resp[0] & MMC_SPI_R1_IDLE) == 0)
                        {
                            break;
                        }
                    }

                    if (status == SUCCESS)
                    {
                        /* Set sector size of 512 using CMD16. */
                        status = mmc_spi_cmd(mmc, MMC_SPI_CMD16, 0, MMC_SPI_SECTOR_SIZE, NULL, 0);
                    }

                    if (status == SUCCESS)
                    {
                        /* This is SDv1 card. */
                        mmc->flags = MMC_SPI_CARD_SD1;
                    }
                }
                else
                {
                    /* Let get card out of idle. */
                    while (status == SUCCESS)
                    {
                        /* Send CMD1. */
                        status = mmc_spi_cmd(mmc, MMC_SPI_CMD1, 0, 0, resp, 1);

                        /* If card is now out of idle. */
                        if ((resp[0] & MMC_SPI_R1_IDLE) == 0)
                        {
                            break;
                        }
                    }

                    if (status == SUCCESS)
                    {
                        /* Set sector size of 512 using CMD16. */
                        status = mmc_spi_cmd(mmc, MMC_SPI_CMD16, 0, MMC_SPI_SECTOR_SIZE, NULL, 0);
                    }

                    if (status == SUCCESS)
                    {
                        /* This is MMCv3 card. */
                        mmc->flags = MMC_SPI_CARD_MMC;
                    }
                }
            }
        }

        /* Unselect the slave. */
        mmc_slave_unselect(mmc);
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_init */

/*
 * mmc_spi_read
 * @mmc: MMC SPI device from which data is needed to be read.
 * @sector: Start sector needed to be read.
 * @offset: On input this should be initialized with 0, on return
 *  will return number of bytes we have read.
 * @buffer: Buffer needed in which data will be read.
 * @size: Size of buffer in bytes.
 * @return: Success will be returned if buffer was successfully read,
 *  MMC_SPI_READ_ERROR will be returned if an error occurred while reading data.
 * This function will read data from the card, this can be called in chucks
 * to read a complete sector or more than one sectors.
 */
int32_t mmc_spi_read(MMC_SPI *mmc, uint32_t sector, uint64_t *offset, uint8_t *buffer, int32_t size)
{
    int32_t this_size, status = SUCCESS;
    uint32_t sector_offset;
    SPI_MSG msg;
    uint8_t line = 0x00;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Need to start a new read transaction. */
    if (*offset == 0)
    {
        /* If card is in byte mode. */
        if ((mmc->flags & MMC_SPI_CARD_BLOCK) == 0)
        {
            /* Convert sector to byte address. */
            sector = sector * MMC_SPI_SECTOR_SIZE;
        }

        /* Send CMD18 on MMC. */
        status = mmc_spi_cmd(mmc, MMC_SPI_CMD18, MMC_SPI_CMD_SEL, sector, NULL, 0);
    }

    /* Calculate current sector offset. */
    sector_offset = (*offset % MMC_SPI_SECTOR_SIZE);

    /* If we need to read data. */
    if (buffer != NULL)
    {
        /* While we have some data to read. */
        while ((status == SUCCESS) && (size != 0))
        {
            /* If we are at the start of a new sector. */
            if (sector_offset == 0)
            {
                /* Initialize a dummy message. */
                msg.buffer = &line;
                msg.length = 1;
                msg.flags = (SPI_MSG_READ);

                /* Wait for card to start data transmission. */
                while (status == SUCCESS)
                {
                    /* Send dummy bytes. */
                    status = mmc->spi.msg(&mmc->spi, &msg);

                    SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "line 0x%02X", line);

                    /* If card has sent us some data. */
                    if (line != 0xFF)
                    {
                        break;
                    }
                }

                /* If this is not data start token. */
                if (line != MMC_SPI_DATA_RX_TKN)
                {
                    /* Return error to the caller. */
                    status = MMC_SPI_READ_ERROR;
                }
            }

            if (status == SUCCESS)
            {
                /* If this read may span on multiple sectors. */
                if ((sector_offset + size) > MMC_SPI_SECTOR_SIZE)
                {
                    /* Just receive number of bytes we can from this sector. */
                    this_size = MMC_SPI_SECTOR_SIZE - sector_offset;
                }
                else
                {
                    /* Send number of bytes given by the caller. */
                    this_size = size;
                }

                /* Initialize message for this data chuck. */
                msg.buffer = buffer;
                msg.length = this_size;
                msg.flags = (SPI_MSG_READ);

                /* Read this chuck. */
                status = mmc->spi.msg(&mmc->spi, &msg);

                if (status == SUCCESS)
                {
                    /* Updated data offset and remaining number of bytes. */
                    *offset += this_size;
                    size -= this_size;
                }
            }

            if (status == SUCCESS)
            {
                /* Update current sector offset. */
                sector_offset = (*offset % MMC_SPI_SECTOR_SIZE);

                /* If a sector is now completely written. */
                if (sector_offset == 0)
                {
                    /* Initialize a dummy message to read CRC. */
                    line = 0xFF;
                    msg.buffer = &line;
                    msg.length = 1;
                    msg.flags = (SPI_MSG_READ);

                    /* Read CRC. */
                    status = mmc->spi.msg(&mmc->spi, &msg);

                    if (status == SUCCESS)
                    {
                        /* Exchange an other dummy byte. */
                        status = mmc->spi.msg(&mmc->spi, &msg);
                    }
                }
            }
        }
    }

    if (status == SUCCESS)
    {
        /* If we have no more data to read. */
        if (buffer == NULL)
        {
            /* Send CMD12 to terminate transaction. */
            status = mmc_spi_cmd(mmc, MMC_SPI_CMD12, 0, sector, NULL, 0);

            if (status == SUCCESS)
            {
                /* Unselect the MMC device. */
                mmc_slave_unselect(mmc);
            }
        }
    }

    if (status != SUCCESS)
    {
        /* Unselect the MMC device. */
        mmc_slave_unselect(mmc);
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_read */

/*
 * mmc_spi_write
 * @mmc: MMC SPI device on which data is needed to be written.
 * @sector: Start sector needed to be written. This is only used to initialize
 *  offset in first call.
 * @offset: At first call this should be initialized with 0, on successive
 *  calls this will track number of bytes we have written.
 * @buffer: Buffer needed to be written.
 * @size: Size of buffer in bytes.
 * @return: Success will be returned if buffer was successfully written,
 *  MMC_SPI_WRITE_ERROR will be returned if an error was detected.
 * This function will write a buffer on the card, this can be called in chucks
 * to transfer a complete sector.
 */
int32_t mmc_spi_write(MMC_SPI *mmc, uint32_t sector, uint64_t *offset, uint8_t *buffer, int32_t size)
{
    int32_t this_size, status = SUCCESS;
    uint32_t sector_offset;
    SPI_MSG msg;
    uint8_t line = 0x00;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Need to start a write transaction. */
    if (*offset == 0)
    {
        /* If card is in byte mode. */
        if ((mmc->flags & MMC_SPI_CARD_BLOCK) == 0)
        {
            /* Convert sector to byte address. */
            sector = sector * MMC_SPI_SECTOR_SIZE;
        }

        /* Send CMD25 on MMC. */
        status = mmc_spi_cmd(mmc, MMC_SPI_CMD25, MMC_SPI_CMD_SEL, sector, NULL, 0);
    }

    /* Calculate current sector offset. */
    sector_offset = (*offset % MMC_SPI_SECTOR_SIZE);

    /* If we have data to transfer. */
    if (buffer != NULL)
    {
        /* If we have more data to transfer. */
        while ((status == SUCCESS) && (size != 0))
        {
            /* If we are at the start of a new sector. */
            if (sector_offset == 0)
            {
                /* Initialize a dummy message. */
                msg.buffer = &line;
                msg.length = 1;
                msg.flags = (SPI_MSG_READ);

                /* Wait for card to get ready. */
                while (status == SUCCESS)
                {
                    /* Read a byte form the SPI. */
                    status = mmc->spi.msg(&mmc->spi, &msg);

                    SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "line 0x%02X", line);

                    /* If SPI line is now stable. */
                    if (line == 0xFF)
                    {
                        break;
                    }
                }

                if (status == SUCCESS)
                {
                    /* Send data token. */
                    line = MMC_SPI_DATA_TX_TKN;

                    /* Write token on the SPI bus. */
                    msg.flags = (SPI_MSG_WRITE);
                    status = mmc->spi.msg(&mmc->spi, &msg);
                }
            }

            if (status == SUCCESS)
            {
                /* If this write may span on multiple sectors. */
                if ((sector_offset + size) > MMC_SPI_SECTOR_SIZE)
                {
                    /* Just send number of bytes we can send in this sector. */
                    this_size = MMC_SPI_SECTOR_SIZE - sector_offset;
                }
                else
                {
                    /* Send number of bytes given by the caller. */
                    this_size = size;
                }

                /* Initialize message for this data chuck. */
                msg.buffer = buffer;
                msg.length = this_size;
                msg.flags = (SPI_MSG_WRITE);

                /* Transfer this chuck. */
                status = mmc->spi.msg(&mmc->spi, &msg);

                if (status == SUCCESS)
                {
                    /* Updated data offset and remaining number of bytes. */
                    *offset += this_size;
                    size -= this_size;
                }
            }

            if (status == SUCCESS)
            {
                /* Update current sector offset. */
                sector_offset = (*offset % MMC_SPI_SECTOR_SIZE);

                /* If a sector is now completely written. */
                if (sector_offset == 0)
                {
                    /* Initialize a dummy message. */
                    line = 0xFF;
                    msg.buffer = &line;
                    msg.length = 1;
                    msg.flags = (SPI_MSG_WRITE);

                    /* Send dummy CRC. */
                    status = mmc->spi.msg(&mmc->spi, &msg);

                    /* If we have transfer anticipated number of bytes. */
                    if (status == msg.length)
                    {
                        /* Reset status. */
                        status = SUCCESS;
                    }

                    if (status == SUCCESS)
                    {
                        /* Read a byte. */
                        msg.flags = (SPI_MSG_READ);

                        /* Exchange a dummy byte. */
                        status = mmc->spi.msg(&mmc->spi, &msg);
                    }

                    if (status == SUCCESS)
                    {
                        /* Read a byte. */
                        msg.flags = (SPI_MSG_READ);

                        /* Read data response. */
                        status = mmc->spi.msg(&mmc->spi, &msg);
                    }

                    if (status == SUCCESS)
                    {
                        SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "DATARSP 0x%02X", line);

                        /* If we encountered an error. */
                        if ((line & 0x1F) != 0x05)
                        {
                            /* Return error to the caller. */
                            status = MMC_SPI_WRITE_ERROR;
                        }
                    }
                }
            }
        }
    }

    if (status == SUCCESS)
    {
        /* If we have no more data to transfer. */
        if (buffer == NULL)
        {
            /* If we are at the start of a new sector. */
            if (sector_offset == 0)
            {
                /* Initialize a dummy message. */
                msg.buffer = &line;
                msg.length = 1;
                msg.flags = (SPI_MSG_READ);

                /* Wait for card to get ready. */
                while (status == SUCCESS)
                {
                    /* Read a byte form the SPI. */
                    status = mmc->spi.msg(&mmc->spi, &msg);

                    SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "line 0x%02X", line);

                    /* If SPI line is now stable. */
                    if (line == 0xFF)
                    {
                        break;
                    }
                }

                if (status == SUCCESS)
                {
                    /* Send data stop token. */
                    line = MMC_SPI_DATA_ST_TKN;

                    /* Write token on the SPI bus. */
                    msg.flags = (SPI_MSG_WRITE);
                    status = mmc->spi.msg(&mmc->spi, &msg);
                }

                if (status == SUCCESS)
                {
                    /* Unselect the MMC device. */
                    mmc_slave_unselect(mmc);
                }
            }
            else
            {
                /* Invalid data write condition detected. */
                status = MMC_SPI_WRITE_ERROR;
            }
        }
    }

    if (status != SUCCESS)
    {
        /* Unselect the MMC device. */
        mmc_slave_unselect(mmc);
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_write */

/*
 * mmc_spi_get_num_sectors
 * @mmc: MMC SPI device for which number of sectors are required.
 * @num_sectors: Number of sectors will be returned here.
 * @return: Success will be returned if number of sectors are successfully
 *  returned.
 * This function will return number of sectors on the card.
 */
int32_t mmc_spi_get_num_sectors(MMC_SPI *mmc, uint64_t *num_sectors)
{
    int32_t status;
    uint32_t drive_size;
    uint8_t csd[MMC_SPI_CSD_LEN], scale;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Read CSD from the device. */
    status = mmc_spi_get_csd(mmc, csd);

    if (status == SUCCESS)
    {
        /* If this is SDv2. */
        if ((csd[0] >> 6) == 1)
        {
            /* Parse the driver size. */
            drive_size = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 0x3F) << 16) + 1;

            /* Return number of sectors. */
            *num_sectors = ((uint64_t)drive_size << 10);
        }
        else
        {
            /* Parse size scale. */
            scale = (csd[5] & 0xF) + ((csd[10] & 0x80) >> 7) + ((csd[9] & 0x3) << 1) + 2;

            /* Parse the driver size. */
            drive_size = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 0x3) << 10) + 1;

            /* Return number of sectors. */
            *num_sectors = ((uint64_t)drive_size << (scale - 9));
        }
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_get_num_sectors */

/*
 * mmc_spi_get_sectors_per_block
 * @mmc: MMC SPI device for which number of sectors per block are
 *  required.
 * @num_sectors: Number of sectors per block will be returned here.
 * @return: Success will be returned if number of sectors per blocks are
 *  successfully returned.
 * This function will return number of sectors per block.
 */
int32_t mmc_spi_get_sectors_per_block(MMC_SPI *mmc, uint64_t *num_sectors)
{
    int32_t status;
    SPI_MSG msg;
    uint8_t csd[MMC_SPI_CSD_LEN], i;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* If this is SDv2 card */
    if (mmc->flags & MMC_SPI_CARD_SD2)
    {
        /* Send CMD55. */
        status = mmc_spi_cmd(mmc, MMC_SPI_CMD55, MMC_SPI_CMD_SEL, 0, NULL, 0);

        if (status == SUCCESS)
        {
            /* Send ACMD51 on MMC. */
            status = mmc_spi_cmd(mmc, MMC_SPI_ACMD51, 0, 0, NULL, 0);

            if (status == SUCCESS)
            {
                /* Receive first 16 SD status bytes. */
                status = mmc_spi_rx_data(mmc, csd, MMC_SPI_CSD_LEN);
            }

            if (status == SUCCESS)
            {
                /* Calculate and save number of sectors per block. */
                *num_sectors = ((uint64_t)16 << (csd[10] >> 4));

                /* Initialize SPI message. */
                msg.buffer = csd;
                msg.length = MMC_SPI_CSD_LEN;
                msg.flags = (SPI_MSG_READ);

                /* Read next 63 bytes [1:63] */
                for (i = MMC_SPI_CSD_LEN; ((status == SUCCESS) && (i < 64)); i += MMC_SPI_CSD_LEN)
                {
                    /* Receive next 16 byte. */
                    status = mmc->spi.msg(&mmc->spi, &msg);
                }
            }

            /* Unselect the slave. */
            mmc_slave_unselect(mmc);
        }
    }
    else
    {
        /* Read CSD. */
        status = mmc_spi_get_csd(mmc, csd);

        if (status == SUCCESS)
        {
            /* If this is SDv1. */
            if (mmc->flags & MMC_SPI_CARD_SD1)
            {
                /* Calculate and return number of sectors per block. */
                *num_sectors = (((csd[10] & 0x3F) << 1) + ((uint64_t)(csd[11] & 0x80) >> 7) + 1) << ((csd[13] >> 6) - 1);
            }

            /* This is MMCv3. */
            else
            {
                /* Calculate and return number of sectors per block. */
                *num_sectors = ((uint64_t)((csd[10] & 0x7C) >> 2) + 1) * (((csd[11] & 0x03) << 3) + ((csd[11] & 0xE0) >> 5) + 1);
            }
        }
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_get_sectors_per_block */

/*
 * mmc_spi_get_csd
 * @mmc: MMC SPI device for which CSD is required.
 * @csd: Read CSD will be returned here.
 * @return: Success will be returned if CSD was successfully read.
 * This function will read CSD of the device.
 */
static int32_t mmc_spi_get_csd(MMC_SPI *mmc, uint8_t *csd)
{
    int32_t status;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Send CMD9 on MMC. */
    status = mmc_spi_cmd(mmc, MMC_SPI_CMD9, MMC_SPI_CMD_SEL, 0x0, NULL, 0);

    if (status == SUCCESS)
    {
        /* Retrieve CSD. */
        status = mmc_spi_rx_data(mmc, csd, MMC_SPI_CSD_LEN);

        /* Unselect the slave. */
        mmc_slave_unselect(mmc);
    }

    if (status == SUCCESS)
    {
        SYS_LOG_FUNTION_HEXDUMP(MMC, SYS_LOG_INFO, "CSD: 0x", csd, 16);
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_get_csd */

/*
 * mmc_spi_rx_data
 * @mmc: MMC SPI device from which data blocks are needed to be read.
 * @buffer: Buffer in which data will be read.
 * @size: Number of bytes needed to be read.
 * @return: Success will be returned if data block was successfully read in
 *  the buffer, MMC_SPI_DATA_RX_TKN will be returned if we did not receive the
 *  anticipated data token.
 * This function will read data from MMC.
 */
static int32_t mmc_spi_rx_data(MMC_SPI *mmc, uint8_t *buffer, int32_t size)
{
    int32_t status = SUCCESS;
    SPI_MSG msg;
    uint8_t line;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Initialize a dummy message. */
    msg.buffer = &line;
    msg.length = 1;
    msg.flags = (SPI_MSG_READ);

    while (status == SUCCESS)
    {
        /* Send dummy bytes. */
        status = mmc->spi.msg(&mmc->spi, &msg);

        SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "line 0x%02X", line);

        if (line != 0xFF)
        {
            break;
        }
    }

    /* If we have a valid data token. */
    if (line == MMC_SPI_DATA_RX_TKN)
    {
        /* Initialize SPI message. */
        msg.buffer = buffer;
        msg.length = size;
        msg.flags = (SPI_MSG_READ);

        /* Receive the data block. */
        status = mmc->spi.msg(&mmc->spi, &msg);
    }
    else
    {
        /* Error while reading data. */
        status = MMC_SPI_READ_ERROR;
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_rx_data */

/*
 * mmc_spi_cmd
 * @mmc: MMC SPI device on which a command a needed to be sent.
 * @cmd: Command needed to be sent.
 * @flags: Command flags,
 *  MMC_SPI_CMD_SEL: if we need to select the device before sending the command,
 *  MMC_SPI_CMD_UNSEL: if device is needed to be unselected after sending this
 *      command.
 * @argv: Command arguments to be sent.
 * @rsp: Buffer in which command response will be read.
 * @rsp_len: Number bytes to read in response.
 * @return: Success will be returned if command was successfully executed,
 *  MMC_SPI_CMD_ERROR will be returned if error status was read from the card.
 * This will execute a command on the given MMC device.
 */
static int32_t mmc_spi_cmd(MMC_SPI *mmc, uint8_t cmd, uint8_t flags, uint32_t argv, uint8_t *rsp, int32_t rsp_len)
{
    int32_t status = SUCCESS;
    SPI_MSG msg;
    uint8_t cmd_buff[MMC_SPI_CMD_LEN];

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* If we need to select the device. */
    if (flags & MMC_SPI_CMD_SEL)
    {
        /* Select the MMC device. */
        status = mmc_slave_select(mmc);
    }

    if (status == SUCCESS)
    {
        /* Initialize command buffer. */
        cmd_buff[0] = (0x40) | cmd;
        cmd_buff[1] = (uint8_t)((argv >> 24) & 0xFF);
        cmd_buff[2] = (uint8_t)((argv >> 16) & 0xFF);
        cmd_buff[3] = (uint8_t)((argv >> 8) & 0xFF);
        cmd_buff[4] = (uint8_t)(argv & 0xFF);

        /* Add CRC only for required commands. */
        switch (cmd)
        {
        /* If we are sending CMD0. */
        case MMC_SPI_CMD0:

            /* For CMD0 use 0x95 as CRC. */
            cmd_buff[5] = 0x95;

            break;

        /* If we are sending CMD8. */
        case MMC_SPI_CMD8:

            /* For CMD8 use 0x87 as CRC. */
            cmd_buff[5] = 0x87;

            break;

        /* Put 0x00 as CRC with end bit as high. */
        default:

            /* Add dummy CRC. */
            cmd_buff[5] = 0x01;

            break;
        }

        /* Initialize SPI message. */
        msg.buffer = cmd_buff;
        msg.length = MMC_SPI_CMD_LEN;
        msg.flags = (SPI_MSG_WRITE);

        /* Send this command. */
        status = mmc->spi.msg(&mmc->spi, &msg);

        /* Initialize dummy SPI message. */
        msg.length = 1;
        msg.flags = (SPI_MSG_READ);

        /* If this is CMD12. */
        if (cmd == MMC_SPI_CMD12)
        {
            /* Read and discard the first byte. */
            mmc->spi.msg(&mmc->spi, &msg);
        }

        while (status == SUCCESS)
        {
            /* Read status byte form SPI. */
            status = mmc->spi.msg(&mmc->spi, &msg);

            SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "status 0x%02X", cmd_buff[0]);

            /* If this is the response byte. */
            if ((cmd_buff[0] & MMC_SPI_R1_COMP) == 0)
            {
                break;
            }
        }

        /* If we can return the status. */
        if ((rsp != NULL) && (rsp_len > 0))
        {
            /* Return the status. */
            rsp[0] = cmd_buff[0];
        }

        SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "CMD: %d, STATUS 0x%02X", cmd, cmd_buff[0]);

        /* If command was successful. */
        if ((cmd_buff[0] & (uint8_t)(~MMC_SPI_R1_IDLE)) == SUCCESS)
        {
            /* If we need to read more response. */
            if ((rsp != NULL) && (rsp_len > 0))
            {
                /* Initialize SPI message. */
                msg.buffer = &rsp[1];
                msg.length = (rsp_len - 1);
                msg.flags = (SPI_MSG_READ);

                /* Send command response. */
                status = mmc->spi.msg(&mmc->spi, &msg);
            }
        }
        else
        {
            /* Return error to the caller. */
            status = MMC_SPI_CMD_ERROR;
        }

        /* If we need to unselect the device. */
        if (flags & MMC_SPI_CMD_UNSEL)
        {
            /* Unselect the slave. */
            mmc_slave_unselect(mmc);
        }
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_spi_cmd */

/*
 * mmc_slave_select
 * @mmc: MMC SPI device for which slave is needed to be selected.
 * @return: Success will be returned if slave was successfully selected.
 * This will select the MMC device.
 */
static int32_t mmc_slave_select(MMC_SPI *mmc)
{
    int32_t status = SUCCESS;
    SPI_MSG msg;
    uint8_t line = 0xFF;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Initialize a dummy message. */
    msg.buffer = &line;
    msg.length = 1;
    msg.flags = (SPI_MSG_READ);

    /* Select the slave. */
    mmc->spi.slave_select(&mmc->spi);

    /* Read a dummy byte form the SPI. */
    mmc->spi.msg(&mmc->spi, &msg);

    while (status == SUCCESS)
    {
        /* Read a byte form the SPI. */
        status = mmc->spi.msg(&mmc->spi, &msg);

        SYS_LOG_FUNTION_MSG(MMC, SYS_LOG_DEBUG, "line 0x%02X", line);

        /* If SPI line is now stable. */
        if (line == 0xFF)
        {
            break;
        }
    }

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_slave_select */

/*
 * mmc_slave_unselect
 * @mmc: MMC SPI device for which slave is needed to be unselected.
 * @return: Success will be returned if slave was successfully unselected.
 * This will unselect the MMC device.
 */
static int32_t mmc_slave_unselect(MMC_SPI *mmc)
{
    int32_t status = SUCCESS;
    SPI_MSG msg;
    uint8_t line;

    SYS_LOG_FUNTION_ENTRY(MMC);

    /* Initialize an SPI message. */
    msg.buffer = &line;
    msg.length = 1;
    msg.flags = (SPI_MSG_READ);

    /* Unselect the slave. */
    mmc->spi.slave_unselect(&mmc->spi);

    /* Read a dummy byte form the SPI. */
    mmc->spi.msg(&mmc->spi, &msg);

    SYS_LOG_FUNTION_EXIT_STATUS(MMC, status);

    /* Return status to the caller. */
    return (status);

} /* mmc_slave_unselect */

#endif /* CONFIG_MMC */
