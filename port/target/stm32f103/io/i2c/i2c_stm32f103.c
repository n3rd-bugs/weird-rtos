/*
 * i2c_stm32f103.c
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

#ifdef CONFIG_I2C
#include <i2c_stm32f103.h>

/*
 * i2c_stm32f103_init
 * @device: I2C device needed to be initialized.
 * This function will initialize a bit-bang I2C device instance.
 */
void i2c_stm32f103_init(I2C_DEVICE *device)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)device->data;
    uint16_t ccr;

    /* Pick the required I2C device. */
    switch (i2c_stm->device_num)
    {

    /* This is I2C1. */
    case 1:

        /* Enable clock for GPIOB. */
        RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

        /* Set alternate function for PB6 (SCL) and PB7 (SDA). */
        GPIOB->CRL &= (uint32_t)(~((0x0F << (6 << 2)) | (0x0F << (7 << 2))));
        GPIOB->CRL |= (uint32_t)(((GPIO_Speed_50MHz | GPIO_Mode_AF_OD) & 0x0F) << (6 << 2));
        GPIOB->CRL |= (uint32_t)(((GPIO_Speed_50MHz | GPIO_Mode_AF_OD) & 0x0F) << (7 << 2));

        /* Reset I2C1. */
        RCC->APB1RSTR |= RCC_APB1Periph_I2C1;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1Periph_I2C1;

        /* Enable clock for I2C1. */
        RCC->APB1ENR |= RCC_APB1Periph_I2C1;

        /* Pick the required device register. */
        i2c_stm->i2c_reg = I2C1;

        break;

    /* This is I2C2. */
    case 2:

        /* Reset I2C2. */
        RCC->APB1RSTR |= RCC_APB1Periph_I2C2;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1Periph_I2C2;

        /* Enable clock for I2C2. */
        RCC->APB1ENR |= RCC_APB1Periph_I2C2;

        /* Pick the required device register. */
        i2c_stm->i2c_reg = I2C2;

        break;

    /* Unknown device number. */
    default:

        /* Nothing to do here. */
        break;
    }

    /* Clear the frequency registers. */
    i2c_stm->i2c_reg->CR2 &= (uint16_t)~(I2C_CR2_FREQ);
    i2c_stm->i2c_reg->CR2 |= (uint16_t)(PCLK_FREQ / 1000000);

    /* Disable I2C device to update CCR and TRISE. */
    i2c_stm->i2c_reg->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_PE);

    /* Set the TRISE register. */
    i2c_stm->i2c_reg->TRISE = (uint16_t)((PCLK_FREQ / 1000000) + 1);

    /* Calculate CCR register value. */
    ccr = (uint16_t)((uint32_t)PCLK_FREQ / (uint32_t)(i2c_stm->speed << 1));

    /* If we have a very low value. */
    if (ccr < 0x4)
    {
        /* Set the minimum allowable value. */
        ccr = 0x4;
    }

    /* Set the CCR register. */
    i2c_stm->i2c_reg->CCR = ccr;

    /* Reset CR1 register. */
    i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_PE | I2C_CR1_SMBUS | I2C_CR1_ENARP | I2C_CR1_ENPEC | I2C_CR1_ENGC | I2C_CR1_NOSTRETCH | I2C_CR1_START | I2C_CR1_STOP | I2C_CR1_POS | I2C_CR1_PEC | I2C_CR1_ALERT | I2C_CR1_SWRST);

    /* Use 7-bit addressing mode. */
    i2c_stm->i2c_reg->OAR1 = 0x00;

    /* Enable I2C device. */
    i2c_stm->i2c_reg->CR1 |= I2C_CR1_PE;

} /* i2c_stm32f103_init */

/*
 * i2c_stm32f103_message
 * @device: I2C device for which messages are needed to be processed.
 * @message: I2C message needed to be sent.
 * @return: Success will be returned if I2C message was successfully processed,
 *  I2C_TIMEOUT will be returned if we timed out waiting for a transition,
 *  I2C_MSG_NACKED will be returned if a byte was NACKed.
 * This function will process a I2C message.
 */
int32_t i2c_stm32f103_message(I2C_DEVICE *device, I2C_MSG *message)
{
    uint64_t timeout;
    int32_t i, status = SUCCESS;
    I2C_STM32 *i2c_stm = (I2C_STM32 *)device->data;

    /* Wait for I2C to become available. */
    timeout = current_hardware_tick();
    while ((((uint32_t)i2c_stm->i2c_reg->SR2 << 16) & I2C_FLAG_BUSY) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
    {
        ;
    }

    /* If we timed out. */
    if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
    {
        /* Return an error to the caller. */
        status = I2C_TIMEOUT;
    }

    if (status == SUCCESS)
    {
        /* Generate a START condition */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_START;

        /* Wait for master mode selection. */
        timeout = current_hardware_tick();
        while (!I2C_STM_IS_EVENT(i2c_stm, I2C_EVENT_MASTER_MODE_SELECT) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
        {
            ;
        }

        /* If we timed out. */
        if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
        {
            /* Return an error to the caller. */
            status = I2C_TIMEOUT;
        }
    }

    if (status == SUCCESS)
    {
        /* Send the I2C address. */
        i2c_stm->i2c_reg->DR = (uint16_t)((device->address << 1) | (message->flags == I2C_MSG_READ));

        /* If we are writing data. */
        if (message->flags == I2C_MSG_WRITE)
        {
            /* Wait for transmitter mode selected. */
            timeout = current_hardware_tick();
            while (!I2C_STM_IS_EVENT(i2c_stm, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
            {
                ;
            }

            /* If we timed out. */
            if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
            {
                /* Return an error to the caller. */
                status = I2C_TIMEOUT;
            }
        }
        else
        {
            /* Wait for receiver mode selected. */
            timeout = current_hardware_tick();
            while (!I2C_STM_IS_EVENT(i2c_stm, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
            {
                ;
            }

            /* If we timed out. */
            if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
            {
                /* Return an error to the caller. */
                status = I2C_TIMEOUT;
            }
        }
    }

    /* Process the message request. */
    switch (message->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
    {

    /* If we are writing. */
    case I2C_MSG_WRITE:

        /* Transfer this message. */
        for (i = 0; ((i < message->length) && (status == SUCCESS)); i++)
        {
            /* Transmit this byte. */
            i2c_stm->i2c_reg->DR = message->buffer[i];

            /* Wait for the byte to be transmitted. */
            timeout = current_hardware_tick();
            while (!I2C_STM_IS_EVENT(i2c_stm, I2C_EVENT_MASTER_BYTE_TRANSMITTING) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
            {
                ;
            }

            /* If we timed out. */
            if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
            {
                /* Return an error to the caller. */
                status = I2C_TIMEOUT;
            }
        }

        break;

    /* If we are reading. */
    case I2C_MSG_READ:

        /* Enable acknowledgments. */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_ACK;

        /* Transfer this message. */
        for (i = 0; ((i < message->length) && (status == SUCCESS)); i++)
        {
            /* Wait for a byte to become available. */
            timeout = current_hardware_tick();
            while (!I2C_STM_IS_EVENT(i2c_stm, I2C_EVENT_MASTER_BYTE_RECEIVED) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
            {
                ;
            }

            /* If we timed out. */
            if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
            {
                /* Return an error to the caller. */
                status = I2C_TIMEOUT;
            }

            if (status == SUCCESS)
            {
                /* Read this byte. */
                message->buffer[i] = (uint8_t)i2c_stm->i2c_reg->DR;
            }
        }

        /* NAK current byte to stop transmission. */
        i2c_stm->i2c_reg->CR1 &= I2C_NACKPosition_Current;

        /* Disable acknowledgments. */
        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

        break;
    }

    /* Generate stop bit. */
    i2c_stm->i2c_reg->CR1 |= I2C_CR1_STOP;

    /* Wait for stop bit. */
    timeout = current_hardware_tick();
    while ((i2c_stm->i2c_reg->SR1 & I2C_FLAG_STOPF) && (HW_TICK_TO_US(current_hardware_tick() - timeout) < STM_I2C_WAIT))
    {
        ;
    }

    /* If we timed out. */
    if (HW_TICK_TO_US(current_hardware_tick() - timeout) > STM_I2C_WAIT)
    {
        /* Return an error to the caller. */
        status = I2C_TIMEOUT;
    }

    /* Return status to the caller. */
    return (status);

} /* i2c_stm32f103_message */
#endif /* CONFIG_I2C */
