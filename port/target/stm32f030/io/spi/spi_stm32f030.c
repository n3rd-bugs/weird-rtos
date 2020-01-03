/*
 * spi_stm32f030.c
 *
 * Copyright (c) 2020 Usama Masood <mirzaon@gmail.com> All rights reserved.
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
#include <spi_stm32f030.h>

/*
 * spi_stm32f030_init
 * @device: SPI device needed to be initialized.
 * This function will initialize a STM32F030 SPI device instance.
 */
void spi_stm32f030_init(SPI_DEVICE *device)
{
    uint32_t baud_scale = 0;
    uint8_t bit_count = 0;

    /* Select the required SPI register and initialize GPIO. */
    switch (((STM32F030_SPI *)device->data)->device_num)
    {
    case 1:
        /* SPI1 device. */
        ((STM32F030_SPI *)device->data)->reg = SPI1;

        /* Reset SPI1. */
        RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
        RCC->APB2RSTR &= (uint32_t)~RCC_APB2RSTR_SPI1RST;

        /* Enable AHB clock for SPI1. */
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

        /* Enable clock for GPIOA. */
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

        /* Set alternate function for PA5 (SCK), PA6 (MISO) and PA7 (MOSI). */
        GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
        GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1);

        /* Select high speed for PA4 (CS), PA5 (SCK), PA6 (MISO) and PA7 (MOSI). */
        GPIOA->OSPEEDR &= (uint32_t)~(GPIO_OSPEEDER_OSPEEDR4 | GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);
        GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR4_1 | GPIO_OSPEEDER_OSPEEDR4_0) | (GPIO_OSPEEDER_OSPEEDR5_1 | GPIO_OSPEEDER_OSPEEDR5_0) | (GPIO_OSPEEDER_OSPEEDR6_1 | GPIO_OSPEEDER_OSPEEDR6_0) | (GPIO_OSPEEDER_OSPEEDR7_1 | GPIO_OSPEEDER_OSPEEDR7_0);

        /* Select SPI1 AF for PA5 (SCK), PA6 (MISO) and PA7 (MOSI). */
        GPIOA->AFR[0] &= (uint32_t)~((0xF << ((5 % 8) << 2)) | (0xF << ((6 % 8) << 2)) | (0xF << ((7 % 8) << 2)));
        GPIOA->AFR[0] |= (0x0 << ((5 % 8) << 2)) | (0x0 << ((6 % 8) << 2)) | (0x0 << ((7 % 8) << 2));

        /* If hardware SS signal is required. */
        if (device->cfg_flags & SPI_CFG_ENABLE_HARD_SS)
        {
            /* Set alternate function for PA4 (SS). */
            GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER4);
            GPIOA->MODER |= (GPIO_MODER_MODER4_1);

            /* Select SPI1 AF for PA4 (SS). */
            GPIOA->AFR[0] &= (uint32_t)~(0xF << ((4 % 8) << 2));
            GPIOA->AFR[0] |= (0x0 << ((5 % 8) << 2));
        }
        else
        {
            /* Select output mode for PA4 (CS). */
            GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER4);
            GPIOA->MODER |= (GPIO_MODER_MODER4_0);

            /* Select pull-up mode for for the PA4 (CS) lines. */
            GPIOA->PUPDR &= (uint32_t)~(GPIO_PUPDR_PUPDR4);
            GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0;
            GPIOA->OTYPER |= GPIO_OTYPER_OT_4;

            /* Set the PA4 (CS). */
            GPIOA->BSRR |= (1 << 4);
        }

        /* Calculate the required baudrate prescaler. */
        baud_scale = CEIL_DIV(PCLK_FREQ, device->baudrate);

        break;

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
    ((STM32F030_SPI *)device->data)->reg->CR1 = (uint16_t)(((uint16_t)(((device->cfg_flags & SPI_CFG_1_WIRE) != 0) << STM32F030_SPI_CR1_BIDI_SHIFT)) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_1_WIRE) != 0) && ((device->cfg_flags & SPI_CFG_RX_ONLY) != 0)) << STM32F030_SPI_CR1_BIDIOE_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_ENABLE_CRC) != 0) << STM32F030_SPI_CR1_CRCEN_SHIFT) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_1_WIRE) == 0) && ((device->cfg_flags & SPI_CFG_RX_ONLY) != 0)) << STM32F030_SPI_CR1_RXONLY_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) == 0) << STM32F030_SPI_CR1_SMM_SHIFT) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) == 0) && ((device->cfg_flags & SPI_CFG_MASTER) != 0)) << STM32F030_SPI_CR1_SSI_SHIFT) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_LSB_FIRST) != 0) << STM32F030_SPI_CR1_LSB_SHIFT)) |
                                                           ((uint16_t)(baud_scale << STM32F030_SPI_CR1_BR_SHIFT)) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_MASTER) != 0) << STM32F030_SPI_CR1_MSTR_SHIFT)) |
                                                           ((uint16_t)(((device->cfg_flags & SPI_CFG_CLK_IDLE_HIGH) != 0) << STM32F030_SPI_CR1_CPOL_SHIFT)) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_CLK_FIRST_DATA) == 0) << STM32F030_SPI_CR1_CPHA_SHIFT));

    /* Put the CR2 register value. */
    ((STM32F030_SPI *)device->data)->reg->CR2 = (uint16_t)((((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) != 0) << STM32F030_SPI_CR2_SSOE_SHIFT) |
                                                           (((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) != 0) << STM32F030_SPI_CR2_NSSP_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_MODE_16BIT) != 0 ? 0xF : 0x7) << STM32F030_SPI_CR2_DS_SHIFT) |
                                                           ((uint16_t)((device->cfg_flags & SPI_CFG_MODE_16BIT) != 0 ? 0x0 : 0x1) << STM32F030_SPI_CR2_FRXTH_SHIFT));

    /* Disable the I2S mode. */
    ((STM32F030_SPI *)device->data)->reg->I2SCFGR &= (uint16_t)(~(1 << STM32F030_SPI_I2SCFG_MOD_SHIFT));

    /* Enable SPI device. */
    ((STM32F030_SPI *)device->data)->reg->CR1 |= (1 << STM32F030_SPI_CR1_SPE_SHIFT);

} /* spi_stm32f030_init */

/*
 * spi_stm32f030_slave_select
 * This function will enable the slave so it can receive data.
 */
void spi_stm32f030_slave_select(SPI_DEVICE *device)
{
    switch (((STM32F030_SPI *)device->data)->device_num)
    {
    /* If this is SPI1. */
    case 1:

        /* Reset the PA4 (CS). */
        GPIOA->BSRR |= (1 << (4 + 16));

        break;
    }

} /* spi_stm32f030_slave_select */

/*
 * spi_stm32f030_slave_unselect
 * This function will disable the slave.
 */
void spi_stm32f030_slave_unselect(SPI_DEVICE *device)
{
    switch (((STM32F030_SPI *)device->data)->device_num)
    {
    /* If this is SPI1. */
    case 1:

        /* Set the PA4 (CS). */
        GPIOA->BSRR |= (1 << 4);

        break;
    }

} /* spi_stm32f030_slave_unselect */

/*
 * spi_stm32f030_message
 * @device: SPI device for which messages are needed to be processed.
 * @message: SPI message needed to be sent.
 * @return: Success will be returned if SPI message was successfully processed.
 * This function will process a SPI message.
 */
int32_t spi_stm32f030_message(SPI_DEVICE *device, SPI_MSG *message)
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
            while (((((STM32F030_SPI *)device->data)->reg->SR & STM32F030_SPI_SR_TXE) == 0) && (timeout++ < STM32F030_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F030_SPI_TIMEOUT)
            {
                /* Send a byte. */
                *(__IO uint8_t *)(&((STM32F030_SPI *)device->data)->reg->DR) = 0xFF;
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
            while (((((STM32F030_SPI *)device->data)->reg->SR & STM32F030_SPI_SR_RXNE) == 0) && (timeout++ < STM32F030_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F030_SPI_TIMEOUT)
            {
                /* Save the data read from the device. */
                byte = (uint8_t)((STM32F030_SPI *)device->data)->reg->DR;

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
            while (((((STM32F030_SPI *)device->data)->reg->SR & STM32F030_SPI_SR_TXE) == 0) && (timeout++ < STM32F030_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F030_SPI_TIMEOUT)
            {
                /* Send a byte. */
                *(__IO uint8_t *)(&((STM32F030_SPI *)device->data)->reg->DR) = message->buffer[bytes];
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
            while (((((STM32F030_SPI *)device->data)->reg->SR & STM32F030_SPI_SR_RXNE) == 0) && (timeout++ < STM32F030_SPI_TIMEOUT));

            /* If we did not timeout for this request. */
            if (timeout < STM32F030_SPI_TIMEOUT)
            {
                /* Save the data read from the device. */
                byte = (uint8_t)((STM32F030_SPI *)device->data)->reg->DR;

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

} /* spi_stm32f030_message */

#endif /* CONFIG_SPI */
