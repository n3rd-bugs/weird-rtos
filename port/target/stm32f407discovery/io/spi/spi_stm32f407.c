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
 * This function will initialize a STM32F407 device instance.
 */
void spi_stm32f407_init(SPI_DEVICE *device)
{
    uint32_t baud_scale;
    uint8_t bit_count = 0;

    /* Select the required SPI register and initialize GPIO. */
    switch (device->data.device_num)
    {
    case 1:
        /* SPI1 device. */
        device->data.reg = SPI1;

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

        /* Configure GPIO speed (50MHz). */
        GPIOA->OSPEEDR &= ~((GPIO_OSPEEDER_OSPEEDR0 << (4 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (5 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (6 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (7 * 2)));
        GPIOA->OSPEEDR |= ((0x02 << (4 * 2)) | (0x02 << (5 * 2)) | (0x02 << (6 * 2)) | (0x02 << (7 * 2)));

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
        device->data.reg = SPI2;

        /* Enable AHB clock for SPI2. */
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

        break;

    case 3:
        /* SPI3 device. */
        device->data.reg = SPI3;

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
    device->data.reg->CR1 = ((uint32_t)((device->cfg_flags & SPI_CFG_1_WIRE) != 0) << STM32F407_SPI_CR1_BIDI_SHIFT) |
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
    device->data.reg->CR2 = (((device->cfg_flags & SPI_CFG_ENABLE_HARD_SS) != 0) << STM32F407_SPI_CR1_SSOE_SHIFT);

    /* Disable the I2S mode. */
    device->data.reg->I2SCFGR &= (uint32_t)(~(1 << STM32F407_SPI_I2SCFG_MOD_SHIFT));

    /* Enable SPI device. */
    device->data.reg->CR1 |= (1 << STM32F407_SPI_CR1_SPE_SHIFT);

} /* spi_stm32f407_init */

/*
 * spi_stm32f407_slave_select
 * This function will enable the slave so it can receive data.
 */
void spi_stm32f407_slave_select(SPI_DEVICE *device)
{
    switch (device->data.device_num)
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
    switch (device->data.device_num)
    {
    case 1:
        /* Set the CS i.e. GPIOA.4. */
        GPIOA->BSRR |= (1 << 4);
    }

} /* spi_stm32f407_slave_unselect */

/*
 * spi_stm32f407_write_read
 * @device: SPI device on which we need to write and then read data.
 * @buffer: Buffer from which data will be written, same buffer will be updated
 *  with the data written.
 * @length: Size of the provided buffer.
 * This function will write and then read data from a SPI device.
 */
int32_t spi_stm32f407_write_read(SPI_DEVICE *device, uint8_t *buffer, int32_t length)
{
    int32_t ret_bytes = length;

    /* While we have a byte to write and read. */
    while (length --)
    {
        /* Wait while TX buffer is not empty. */
        while ((device->data.reg->SR & STM32F407_SPI_SR_TXE) == 0);

        /* Send a byte. */
        device->data.reg->DR = *buffer;

        /* Wait while we don't have any data to read. */
        while ((device->data.reg->SR & STM32F407_SPI_SR_RXNE) == 0);

        /* Save the data read from the device. */
        *buffer = (uint8_t)device->data.reg->DR;

        /* Get next byte to send and update. */
        buffer++;
    }

    /* Return number of bytes written to and read from the SPI. */
    return (ret_bytes);

} /* spi_stm32f407_write_read */

#endif /* CONFIG_SPI */
