/*
 * spi_stm32f407.c
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
#include <os.h>

#ifdef CONFIG_SPI
#include <spi.h>
#include <spi_stm32f407.h>

/*
 * spi_stm32f407_init
 * @device: SPI device needed to be initialized.
 * This function will initialize a STM32F407 SPI device instance.
 */
void spi_stm32f407_init(SPI_DEVICE *device)
{
    uint32_t baud_scale;
    uint8_t bit_count = 0;

    /* Select the required SPI register and initialize GPIO. */
    switch (((STM32F407_SPI *)device->data)->device_num)
    {
    case 1:
        /* SPI1 device. */
        ((STM32F407_SPI *)device->data)->reg = SPI1;

        /* Enable AHB clock for SPI1. */
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

        /* Enable GPIO A clock. */
        RCC->AHB1ENR |= 0x00000001;

        /* Configure GPIO mode output for GPIOA.4 and alternate function for GPIOA.5, GPIOA.6, GPIOA.7. */
        GPIOA->MODER &= ~((GPIO_MODER_MODER0 << (4 * 2)) | (GPIO_MODER_MODER0 << (5 * 2)) | (GPIO_MODER_MODER0 << (6 * 2)) | (GPIO_MODER_MODER0 << (7 * 2)));
        GPIOA->MODER |= ((0x01 << (4 * 2)) | (0x02 << (5 * 2)) | (0x02 << (6 * 2)) | (0x02 << (7 * 2)));

        /* Configure output type (PP) for GPIOA.4, GPIOA.5, GPIOA.6, GPIOA.7. */
        GPIOA->OTYPER &= ~((GPIO_OTYPER_OT_0 << (4 * 2)) | (GPIO_OTYPER_OT_0 << (5 * 2)) | (GPIO_OTYPER_OT_0 << (6 * 2)) | (GPIO_OTYPER_OT_0 << (7 * 2)));

        /* Enable pull-down on GPIOA.5, GPIOA.6, GPIOA.7. */
        GPIOA->PUPDR &= ~((GPIO_PUPDR_PUPDR0 << (4 * 2)) | (GPIO_PUPDR_PUPDR0 << (5 * 2)) | (GPIO_PUPDR_PUPDR0 << (6 * 2)) | (GPIO_PUPDR_PUPDR0 << (7 * 2)));
        GPIOA->PUPDR |= ((0x02 << (5 * 2)) | (0x02 << (6 * 2)) | (0x02 << (7 * 2)));

        /* Configure GPIO speed (100MHz). */
        GPIOA->OSPEEDR &= ~((GPIO_OSPEEDER_OSPEEDR0 << (4 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (5 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (6 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (7 * 2)));
        GPIOA->OSPEEDR |= ((0x03 << (4 * 2)) | (0x03 << (5 * 2)) | (0x03 << (6 * 2)) | (0x03 << (7 * 2)));

        /* Enable SPI mode on GPIOA.5. */
        GPIOA->AFR[0x05 >> 0x03] &= (uint32_t)(~(0xF << ((0x05 & 0x07) * 4)));
        GPIOA->AFR[0x05 >> 0x03] |= 0x5 << ((0x05 & 0x07) * 4);

        /* Enable SPI mode on GPIOA.6. */
        GPIOA->AFR[0x06 >> 0x03] &= (uint32_t)(~(0xF << ((0x06 & 0x07) * 4)));
        GPIOA->AFR[0x06 >> 0x03] |= 0x5 << ((0x06 & 0x07) * 4);

        /* Enable SPI mode on GPIOA.7. */
        GPIOA->AFR[0x07 >> 0x03] &= (uint32_t)(~(0xF << ((0x07 & 0x07) * 4)));
        GPIOA->AFR[0x07 >> 0x03] |= 0x5 << ((0x07 & 0x07) * 4);

        /* Set the CS. */
        GPIOA->BSRR |= (1 << 4);

        break;

    case 2:
        /* SPI2 device. */
        ((STM32F407_SPI *)device->data)->reg = SPI2;

        /* Enable AHB clock for SPI2. */
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

        break;

    case 3:
        /* SPI3 device. */
        ((STM32F407_SPI *)device->data)->reg = SPI3;

        /* Enable AHB clock for SPI3. */
        RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

        break;

    default:
        /* Invalid SPI device. */
        OS_ASSERT(TRUE);

        break;
    }

    /* Calculate the required baudrate prescaler. */
    baud_scale = PCLK_FREQ / device->baudrate;
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
    ((STM32F407_SPI *)device->data)->reg->CR1 = ((uint32_t)((device->cfg_flags & SPI_CFG_1_WIRE) != 0) << STM32F407_SPI_CR1_BIDI_SHIFT) |
                                                ((uint32_t)(((device->cfg_flags & SPI_CFG_1_WIRE) != 0) && ((device->cfg_flags & SPI_CFG_RX_ONLY) != 0)) << STM32F407_SPI_CR1_BIDIOE_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_ENABLE_CRC) != 0) << STM32F407_SPI_CR1_CRCEN_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_MODE_16BIT) != 0) << STM32F407_SPI_CR1_DFF_SHIFT) |
                                                ((uint32_t)(((device->cfg_flags & SPI_CFG_1_WIRE) == 0) && ((device->cfg_flags & SPI_CFG_RX_ONLY) != 0)) << STM32F407_SPI_CR1_RXONLY_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) == 0) << STM32F407_SPI_CR1_SMM_SHIFT) |
                                                ((uint32_t)(((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) == 0) && ((device->cfg_flags & SPI_CFG_MASTER) != 0)) << STM32F407_SPI_CR1_SSI_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_LSB_FIRST) != 0) << STM32F407_SPI_CR1_LSB_SHIFT) |
                                                (baud_scale << STM32F407_SPI_CR1_BR_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_MASTER) != 0) << STM32F407_SPI_CR1_MSTR_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_CLK_IDLE_HIGH) != 0) << STM32F407_SPI_CR1_CPOL_SHIFT) |
                                                ((uint32_t)((device->cfg_flags & SPI_CFG_CLK_FIRST_DATA) == 0) << STM32F407_SPI_CR1_CPHA_SHIFT);

    /* Put the CR2 register value. */
    ((STM32F407_SPI *)device->data)->reg->CR2 = (((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) != 0) << STM32F407_SPI_CR1_SSOE_SHIFT);

    /* Disable the I2S mode. */
    ((STM32F407_SPI *)device->data)->reg->I2SCFGR &= (uint32_t)(~(1 << STM32F407_SPI_I2SCFG_MOD_SHIFT));

    /* Enable SPI device. */
    ((STM32F407_SPI *)device->data)->reg->CR1 |= (1 << STM32F407_SPI_CR1_SPE_SHIFT);

} /* spi_stm32f407_init */

/*
 * spi_stm32f407_slave_select
 * This function will enable the slave so it can receive data.
 */
void spi_stm32f407_slave_select(SPI_DEVICE *device)
{
    switch (((STM32F407_SPI *)device->data)->device_num)
    {
    case 1:
        /* Reset the CS i.e. GPIOA.4. */
        GPIOA->BSRR |= (1 << (4 + 16));
    }

} /* spi_stm32f407_slave_select */

/*
 * spi_stm32f407_slave_unselect
 * This function will disable the slave.
 */
void spi_stm32f407_slave_unselect(SPI_DEVICE *device)
{
    switch (((STM32F407_SPI *)device->data)->device_num)
    {
    case 1:
        /* Set the CS i.e. GPIOA.4. */
        GPIOA->BSRR |= (1 << 4);
    }

} /* spi_stm32f407_slave_unselect */

/*
 * spi_stm32f407_message
 * @device: SPI device for which messages are needed to be processed.
 * @message: SPI message needed to be sent.
 * @return: Success will be returned if SPI message was successfully processed.
 * This function will process a SPI message.
 */
int32_t spi_stm32f407_message(SPI_DEVICE *device, SPI_MSG *message)
{
    int32_t bytes = 0, timeout;
    uint8_t byte;

    /* While we have a byte to write and read. */
    while (bytes < message->length)
    {
        /* Wait while TX buffer is not empty. */
        timeout = 0;
        while (((((STM32F407_SPI *)device->data)->reg->SR & STM32F407_SPI_SR_TXE) == 0) && (timeout++ < STM32F407_SPI_TIMEOUT));

        /* If we did not timeout for this request. */
        if (timeout < STM32F407_SPI_TIMEOUT)
        {
            /* Send a byte. */
            ((STM32F407_SPI *)device->data)->reg->DR = message->buffer[bytes];
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
        while (((((STM32F407_SPI *)device->data)->reg->SR & STM32F407_SPI_SR_RXNE) == 0) && (timeout++ < STM32F407_SPI_TIMEOUT));

        /* If we did not timeout for this request. */
        if (timeout < STM32F407_SPI_TIMEOUT)
        {
            /* Save the data read from the device. */
            byte = (uint8_t)((STM32F407_SPI *)device->data)->reg->DR;

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

    /* If we did not encounter any error. */
    if (bytes > 0)
    {
        /* Return status to the caller. */
        bytes = SUCCESS;
    }

    /* Return status to the caller. */
    return (bytes);

} /* spi_stm32f407_message */

#endif /* CONFIG_SPI */
