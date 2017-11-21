/*
 * spi_stm32f103.c
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

#ifdef CONFIG_SPI
#include <spi.h>
#include <spi_stm32f103.h>

/*
 * spi_stm32f103_init
 * @device: SPI device needed to be initialized.
 * This function will initialize a STM32F103 SPI device instance.
 */
void spi_stm32f103_init(SPI_DEVICE *device)
{
    uint32_t baud_scale = 0;
    uint8_t bit_count = 0;

    /* Select the required SPI register and initialize GPIO. */
    switch (((STM32F103_SPI *)device->data)->device_num)
    {
    case 1:
        /* SPI1 device. */
        ((STM32F103_SPI *)device->data)->reg = SPI1;

        /* Reset SPI1. */
        RCC->APB2RSTR |= RCC_APB2Periph_SPI1;
        RCC->APB2RSTR &= (uint32_t)~RCC_APB2Periph_SPI1;

        /* Enable AHB clock for SPI1. */
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

        /* Enable clock for GPIOA. */
        RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

        /* Set alternate function for PA5 (SCLK), PA6 (MISO) and PA7 (MOSI). */
        GPIOA->CRL &= (uint32_t)(~((0x0F << (5 << 2)) | (0x0F << (6 << 2)) | (0x0F << (7 << 2))));
        GPIOA->CRL |= (uint32_t)((((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << (5 << 2)) | (((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << (6 << 2)) | (((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << (7 << 2)));

        /* Set PA4 (NSS) as output. */
        GPIOA->CRL &= (uint32_t)(~(0x0F << (4 << 2)));
        GPIOA->CRL |= (((GPIO_Speed_50MHz | GPIO_Mode_Out_PP) & 0x0F) << (4 << 2));

        /* Set the CS. */
        GPIOA->BSRR |= (1 << 4);

        /* Calculate the required baudrate prescaler. */
        baud_scale = CEIL_DIV(PCLK2_FREQ, device->baudrate);

        break;

    case 2:
        /* SPI2 device. */
        ((STM32F103_SPI *)device->data)->reg = SPI2;

        /* Reset SPI2. */
        RCC->APB1RSTR |= RCC_APB1Periph_SPI2;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1Periph_SPI2;

        /* Enable AHB clock for SPI2. */
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

        /* Enable clock for GPIOB. */
        RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

        /* Set alternate function for PB13 (SCLK), PB14 (MISO) and PB15 (MOSI). */
        GPIOB->CRH &= (uint32_t)(~((0x0F << ((13 - 8) << 2)) | (0x0F << ((14 - 8) << 2)) | (0x0F << ((15 - 8) << 2))));
        GPIOB->CRH |= (uint32_t)((((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << ((13 - 8) << 2)) | (((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << ((14 - 8) << 2)) | (((GPIO_Speed_50MHz | GPIO_Mode_AF_PP) & 0x0F) << ((15 - 8) << 2)));

        /* Set PB12 (NSS) as output. */
        GPIOB->CRH &= (uint32_t)(~(0x0F << ((12 - 8) << 2)));
        GPIOB->CRH |= (((GPIO_Speed_50MHz | GPIO_Mode_Out_PP) & 0x0F) << ((12 - 8) << 2));

        /* Set the PB12 (CS). */
        GPIOB->BSRR |= (1 << 12);

        /* Calculate the required baudrate prescaler. */
        baud_scale = CEIL_DIV(PCLK1_FREQ, device->baudrate);

        break;

#if defined (STM32F10X_HD) || defined  (STM32F10X_CL)
    case 3:
        /* SPI3 device. */
        ((STM32F103_SPI *)device->data)->reg = SPI3;

        /* Reset SPI3. */
        RCC->APB1RSTR |= RCC_APB1Periph_SPI3;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1Periph_SPI3;

        /* Enable AHB clock for SPI3. */
        RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

        /* Calculate the required baudrate prescaler. */
        baud_scale = CEIL_DIV(PCLK1_FREQ, device->baudrate);

        break;

#endif /* defined (STM32F10X_HD) || defined  (STM32F10X_CL) */

    default:
        /* Invalid SPI device. */
        ASSERT(TRUE);

        break;
    }

    if (baud_scale >= 256)
    {
        /* Use baud scale 7. */
        baud_scale = 7;
    }
    else if (baud_scale <= 2)
    {
        /* Use baud scale 0. */
        baud_scale = 0;
    }
    else
    {
        /* Calculate the number of bits in the scale. */
        while (baud_scale)
        {
            baud_scale = baud_scale >> 1;
            bit_count++;
        }

        /* Calculate the required baud scale. */
        baud_scale = (uint32_t)(bit_count - 2);
    }

    /* Put the CR1 register value. */
    ((STM32F103_SPI *)device->data)->reg->CR1 = (uint16_t)(((uint16_t)(((device->cfg_flags & SPI_CFG_1_WIRE) != 0) << STM32F103_SPI_CR1_BIDI_SHIFT)) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_1_WIRE) != 0) && ((device->cfg_flags & SPI_CFG_RX_ONLY) != 0)) << STM32F103_SPI_CR1_BIDIOE_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_ENABLE_CRC) != 0) << STM32F103_SPI_CR1_CRCEN_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_MODE_16BIT) != 0) << STM32F103_SPI_CR1_DFF_SHIFT) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_1_WIRE) == 0) && ((device->cfg_flags & SPI_CFG_RX_ONLY) != 0)) << STM32F103_SPI_CR1_RXONLY_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) == 0) << STM32F103_SPI_CR1_SMM_SHIFT) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) == 0) && ((device->cfg_flags & SPI_CFG_MASTER) != 0)) << STM32F103_SPI_CR1_SSI_SHIFT) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_LSB_FIRST) != 0) << STM32F103_SPI_CR1_LSB_SHIFT)) |
                                                           ((uint16_t)(baud_scale << STM32F103_SPI_CR1_BR_SHIFT)) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_MASTER) != 0) << STM32F103_SPI_CR1_MSTR_SHIFT)) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_CLK_IDLE_HIGH) != 0) << STM32F103_SPI_CR1_CPOL_SHIFT)) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_CLK_FIRST_DATA) == 0) << STM32F103_SPI_CR1_CPHA_SHIFT));

    /* Put the CR2 register value. */
    ((STM32F103_SPI *)device->data)->reg->CR2 = (((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) != 0) << STM32F103_SPI_CR1_SSOE_SHIFT);

    /* Disable the I2S mode. */
    ((STM32F103_SPI *)device->data)->reg->I2SCFGR &= (uint16_t)(~(1 << STM32F103_SPI_I2SCFG_MOD_SHIFT));

    /* Enable SPI device. */
    ((STM32F103_SPI *)device->data)->reg->CR1 |= (1 << STM32F103_SPI_CR1_SPE_SHIFT);

} /* spi_stm32f103_init */

/*
 * spi_stm32f103_slave_select
 * This function will enable the slave so it can receive data.
 */
void spi_stm32f103_slave_select(SPI_DEVICE *device)
{
    switch (((STM32F103_SPI *)device->data)->device_num)
    {
    /* If this is SPI1. */
    case 1:

        /* Reset the PA4 (CS). */
        GPIOA->BSRR |= (1 << (4 + 16));

        break;

    /* If this is SPI2. */
    case 2:

        /* Reset the PB12 (CS). */
        GPIOB->BSRR |= (1 << (12 + 16));

        break;
    }

} /* spi_stm32f103_slave_select */

/*
 * spi_stm32f103_slave_unselect
 * This function will disable the slave.
 */
void spi_stm32f103_slave_unselect(SPI_DEVICE *device)
{
    switch (((STM32F103_SPI *)device->data)->device_num)
    {
    /* If this is SPI1. */
    case 1:

        /* Set the PA4 (CS). */
        GPIOA->BSRR |= (1 << 4);

        break;

    /* If this is SPI2. */
    case 2:

        /* Reset the PB12 (CS). */
        GPIOB->BSRR |= (1 << 12);

        break;
    }

} /* spi_stm32f103_slave_unselect */

/*
 * spi_stm32f103_message
 * @device: SPI device for which messages are needed to be processed.
 * @message: SPI message needed to be sent.
 * @return: Success will be returned if SPI message was successfully processed.
 * This function will process a SPI message.
 */
int32_t spi_stm32f103_message(SPI_DEVICE *device, SPI_MSG *message)
{
    int32_t bytes = 0, timeout;
    uint8_t byte;

    /* Process the message request. */
    switch (message->flags & (SPI_MSG_WRITE | SPI_MSG_READ))
    {
    /* If we are only reading. */
    case SPI_MSG_READ:

        /* While we have a byte to read. */
        while (bytes < message->length)
        {
            /* Wait while TX buffer is not empty. */
            timeout = 0;
            while (((((STM32F103_SPI *)device->data)->reg->SR & STM32F103_SPI_SR_TXE) == 0) && (timeout++ < STM32F103_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F103_SPI_TIMEOUT)
            {
                /* Send a byte. */
                ((STM32F103_SPI *)device->data)->reg->DR = 0xFF;
            }
            else
            {
                /* Return error to the caller. */
                bytes = SPI_TIMEOUT;

                /* Stop processing any more data for this message. */
                break;
            }

            /* Wait while we don't have any data to read. */
            timeout = 0;
            while (((((STM32F103_SPI *)device->data)->reg->SR & STM32F103_SPI_SR_RXNE) == 0) && (timeout++ < STM32F103_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F103_SPI_TIMEOUT)
            {
                /* Save the data read from the device. */
                byte = (uint8_t)((STM32F103_SPI *)device->data)->reg->DR;

                /* Save the byte read from SPI. */
                message->buffer[bytes] = byte;

                /* Get next byte to send and update. */
                bytes++;
            }
            else
            {
                /* Return error to the caller. */
                bytes = SPI_TIMEOUT;

                /* Stop processing any more data for this message. */
                break;
            }
        }

        break;

    /* Either writing, or writing while reading. */
    default:

        /* While we have a byte to write and read. */
        while (bytes < message->length)
        {
            /* Wait while TX buffer is not empty. */
            timeout = 0;
            while (((((STM32F103_SPI *)device->data)->reg->SR & STM32F103_SPI_SR_TXE) == 0) && (timeout++ < STM32F103_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F103_SPI_TIMEOUT)
            {
                /* Send a byte. */
                ((STM32F103_SPI *)device->data)->reg->DR = message->buffer[bytes];
            }
            else
            {
                /* Return error to the caller. */
                bytes = SPI_TIMEOUT;

                /* Stop processing any more data for this message. */
                break;
            }

            /* Wait while we don't have any data to read. */
            timeout = 0;
            while (((((STM32F103_SPI *)device->data)->reg->SR & STM32F103_SPI_SR_RXNE) == 0) && (timeout++ < STM32F103_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F103_SPI_TIMEOUT)
            {
                /* Save the data read from the device. */
                byte = (uint8_t)((STM32F103_SPI *)device->data)->reg->DR;

                /* Check if we are also reading. */
                if (message->flags & SPI_MSG_READ)
                {
                    /* Save the byte read from SPI. */
                    message->buffer[bytes] = byte;
                }

                /* Get next byte to send and update. */
                bytes++;
            }
            else
            {
                /* Return error to the caller. */
                bytes = SPI_TIMEOUT;

                /* Stop processing any more data for this message. */
                break;
            }
        }

        break;
    }

    /* If we did not encounter any error. */
    if (bytes > 0)
    {
        /* Return status to the caller. */
        bytes = SUCCESS;
    }

    /* Return status to the caller. */
    return (bytes);

} /* spi_stm32f103_message */

#endif /* CONFIG_SPI */
