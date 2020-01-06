/*
 * i2c_stm32f103.c
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

#ifdef CONFIG_I2C
#include <i2c_stm32f103.h>

#ifdef STM_I2C_INT_MODE
/* I2C device data. */
static I2C_DEVICE *i2c1_data, *i2c2_data;

/* Internal function prototypes. */
static void i2c1_stm32f103_enable_interrupt(void *);
static void i2c2_stm32f103_disable_interrupt(void *);
#endif /* STM_I2C_INT_MODE */

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
        GPIOB->CRL |= (uint32_t)((((GPIO_Speed_50MHz | GPIO_Mode_AF_OD) & 0x0F) << (6 << 2)) | (((GPIO_Speed_50MHz | GPIO_Mode_AF_OD) & 0x0F) << (7 << 2)));

        /* Reset I2C1. */
        RCC->APB1RSTR |= RCC_APB1Periph_I2C1;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1Periph_I2C1;

        /* Enable clock for I2C1. */
        RCC->APB1ENR |= RCC_APB1Periph_I2C1;

        /* Pick the required device register. */
        i2c_stm->i2c_reg = I2C1;

#ifdef STM_I2C_INT_MODE
        /* We should not register same device again with different data. */
        ASSERT((i2c1_data != NULL) && (i2c1_data != device));

        /* Save data for I2C1. */
        i2c1_data = device;
#endif /* STM_I2C_INT_MODE */

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

#ifdef STM_I2C_INT_MODE
        /* We should not register same device again with different data. */
        ASSERT((i2c2_data != NULL) && (i2c2_data != device));

        /* Save data for I2C2. */
        i2c2_data = device;
#endif /* STM_I2C_INT_MODE */

        break;

    /* Unknown device number. */
    default:

        /* Nothing to do here. */
        break;
    }

    /* Calculate CCR register value. */
    ccr = (uint16_t)((uint32_t)PCLK1_FREQ / (uint32_t)(i2c_stm->speed << 1));

    /* If we have a very low value. */
    if (ccr < 0x4)
    {
        /* Set the minimum allowable value. */
        ccr = 0x4;
    }

    /* Disable I2C device and set the ACK bit. */
    i2c_stm->i2c_reg->CR1 = I2C_CR1_ACK;

    /* Configure I2C clock and enable error interrupts. */
    i2c_stm->i2c_reg->CR2 = (PCLK1_FREQ / 1000000);

    /* Update the CCR register. */
    i2c_stm->i2c_reg->CCR = ccr;

    /* Set the TRISE register. */
    i2c_stm->i2c_reg->TRISE = (PCLK1_FREQ / 1000000) + 1;

    /* Enable 7-bit addressing. */
    i2c_stm->i2c_reg->OAR1 = 0x00;

    /* Enable device. */
    i2c_stm->i2c_reg->CR1 |= I2C_CR1_PE;

    /* Test if device is in busy state. */
    if (i2c_stm->i2c_reg->SR2 & I2C_SR2_BUSY)
    {
        /* Power down the device. */
        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_PE);

        /* Process I2C pins according to the selected device. */
        switch (i2c_stm->device_num)
        {

        /* This is I2C1. */
        case 1:
            /* Set SCL/SDA as output, open-drain with 50Mhz. */
            GPIOB->CRL &= (uint32_t)(~((0x0F << (6 << 2)) | (0x0F << (7 << 2))));
            GPIOB->CRL |= ((0x7 << (6 * 4)) | (0x7 << (7 * 4)));

            /* Toggle both pins. */
            GPIOB->ODR |= ((1 << 6) | (1 << 7));
            GPIOB->ODR &= (uint32_t)~((1 << 6) | (1 << 7));

            /* Bring then high again. */
            GPIOB->ODR |= ((1 << 6) | (1 << 7));

            /* Configure back to alternate function. */
            GPIOB->CRL |= (uint32_t)((0x0F << (6 << 2)) | (0x0F << (7 << 2)));

            break;

        /* Unknown device number. */
        default:

            /* Nothing to do here. */
            break;
        }

        /* Software reset the device. */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_SWRST;
        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_SWRST);

        /* Disable I2C device and set the ACK bit. */
        i2c_stm->i2c_reg->CR1 = I2C_CR1_ACK;

        /* Configure I2C clock. */
        i2c_stm->i2c_reg->CR2 = (PCLK1_FREQ / 1000000);

        /* Update the CCR register. */
        i2c_stm->i2c_reg->CCR = ccr;

        /* Set the TRISE register. */
        i2c_stm->i2c_reg->TRISE = (PCLK1_FREQ / 1000000) + 1;

        /* Enable 7-bit addressing. */
        i2c_stm->i2c_reg->OAR1 = 0x00;

        /* Enable device. */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_PE;
    }

#ifdef STM_I2C_INT_MODE
    /* Enable I2C interrupts. */
    i2c1_stm32f103_enable_interrupt(device);

    /* Initialize I2C condition data. */
    i2c_stm->condition.lock = &i2c2_stm32f103_disable_interrupt;
    i2c_stm->condition.unlock = &i2c1_stm32f103_enable_interrupt;
    i2c_stm->condition.data = device;

#endif /* STM_I2C_INT_MODE */

} /* i2c_stm32f103_init */

/*
 * i2c_stm32f103_message
 * @device: I2C device for which messages are needed to be processed.
 * @message: I2C message needed to be sent.
 * @return: Success will be returned if I2C message was successfully processed,
 *  I2C_TIMEOUT will be returned if we timeout in waiting for an event,
 *  I2C_IO_ERROR will be returned if a I2C bus error was detected,
 *  I2C_MSG_NACKED will be returned if a byte was NACKed.
 * This function will process a I2C message.
 */
int32_t i2c_stm32f103_message(I2C_DEVICE *device, I2C_MSG *message)
{
    int32_t status = SUCCESS;
    I2C_STM32 *i2c_stm = (I2C_STM32 *)device->data;
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();
#ifdef STM_I2C_INT_MODE
    CONDITION *condition = &i2c_stm->condition;
    SUSPEND *suspend = &i2c_stm->suspend;
#else
    int32_t i;
#endif /* STM_I2C_INT_MODE */

    /* Wait for I2C to get out of busy state. */
    I2C_STM_TIMED(i2c_stm->i2c_reg->SR2 & I2C_SR2_BUSY);

    if (status == SUCCESS)
    {
#ifdef STM_I2C_INT_MODE
        /* Initialize I2C bus data. */
        i2c_stm->msg = message;
        i2c_stm->bytes_transfered = 0;
        i2c_stm->flags = 0;

        /* Disable interrupts. */
        DISABLE_INTERRUPTS();

        /* Enable event and error interrupts for this I2C. */
        i2c_stm->i2c_reg->CR2 |= (I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);

        /* Send a start bit. */
        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_START | I2C_CR1_ACK);

        /* Enable buffer interrupt for this I2C. */
        i2c_stm->i2c_reg->CR2 |= (I2C_CR2_ITBUFEN);

        /* Enable timeout for suspend. */
        i2c_stm->suspend.timeout_enabled = TRUE;
        i2c_stm->suspend.timeout = current_system_tick() + MS_TO_TICK(STM_I2C_INT_TIMEOUT);
        i2c_stm->suspend.priority = SUSPEND_MIN_PRIORITY;
        i2c_stm->suspend.status = SUCCESS;

        /* Suspend on the I2C interrupts to process this message. */
        status = suspend_condition(&condition, &suspend, NULL, FALSE);

        /* Disable buffer, event and error interrupts for this I2C. */
        i2c_stm->i2c_reg->CR2 &= (uint16_t)~(I2C_CR2_ITBUFEN | I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);

        if (status == SUCCESS)
        {
            /* If a bus error was detected. */
            if (i2c_stm->flags & I2C_STM32_ERROR)
            {
                /* Reinitialize this device to recover from error state. */
                i2c_stm32f103_init(device);

                /* Return IO error to the caller. */
                status = I2C_IO_ERROR;
            }

            /* If remote sent a NAK. */
            else if (i2c_stm->flags & I2C_STM32_NACK)
            {
                /* Return NAK error to the caller. */
                status = I2C_MSG_NACKED;
            }
        }

        /* If we timeout waiting for this transaction. */
        else if (status == CONDITION_TIMEOUT)
        {
            /* Return timeout error to the caller. */
            status = I2C_TIMEOUT;
        }
#else

        /* Send a start bit. */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_START;

        /* Wait for start bit to transmit. */
        I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_SB));

        if (status == SUCCESS)
        {
            /* Send the address byte. */
            i2c_stm->i2c_reg->DR = (uint16_t)((device->address << 1) | (message->flags == I2C_MSG_READ));

            /* Wait for address byte to be sent. */
            I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_ADDR));
        }

        if (status == SUCCESS)
        {
            /* Process the message request. */
            switch (message->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
            {

            /* If we are writing. */
            case I2C_MSG_WRITE:

                /* Clear the address bit. */
                i2c_stm->i2c_reg->SR1;
                i2c_stm->i2c_reg->SR2;

                /* Transfer all the bytes in this message. */
                for (i = 0; ((i < message->length) && (status == SUCCESS)); i++)
                {
                    /* Send this byte. */
                    i2c_stm->i2c_reg->DR = (uint8_t)message->buffer[i];

                    /* Wait for byte transmission. */
                    I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_TXE));
                }

                if (status == SUCCESS)
                {
                    /* Wait for last byte to be sent. */
                    I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_BTF));

                    /* Send the stop bit. */
                    i2c_stm->i2c_reg->CR1 |= I2C_CR1_STOP;
                }

                break;

            /* If we are reading. */
            case I2C_MSG_READ:

                /* Process the message according to the number of bytes to be sent. */
                switch (message->length)
                {

                /* If only one byte is needed to be sent. */
                case 1:

                    /* Clear the ACK bit. */
                    i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    /* Clear the address. */
                    i2c_stm->i2c_reg->SR1;
                    i2c_stm->i2c_reg->SR2;

                    /* We will stop right after this byte. */
                    i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);

                    /* Wait for a byte to be received. */
                    I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_RXNE));

                    if (status == SUCCESS)
                    {
                        /* Read this byte. */
                        message->buffer[0] = (uint8_t)i2c_stm->i2c_reg->DR;

                        /* Wait for transmission to stop. */
                        I2C_STM_TIMED(i2c_stm->i2c_reg->CR1 & I2C_CR1_STOP);
                    }

                    if (status == SUCCESS)
                    {
                        /* Enable ACK to receive more data. */
                        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_ACK);
                    }

                    break;

                /* If we need to receive two bytes byte. */
                case 2:

                    /* Acknowledge this byte. */
                    i2c_stm->i2c_reg->CR1 |= (I2C_CR1_POS | I2C_CR1_ACK);

                    /* Disable global interrupts. */
                    DISABLE_INTERRUPTS();

                    /* Clear the address. */
                    i2c_stm->i2c_reg->SR1;
                    i2c_stm->i2c_reg->SR2;

                    /* Disable ACK. */
                    i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

                    /* Restore old interrupt level. */
                    SET_INTERRUPT_LEVEL(interrupt_level);

                    /* Wait for last byte to be sent. */
                    I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_BTF));

                    if (status == SUCCESS)
                    {
                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* Trigger stop. */
                        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

                        /* Receive first byte. */
                        message->buffer[0] = (uint8_t)i2c_stm->i2c_reg->DR;

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);

                        /* Receive second byte. */
                        message->buffer[1] = (uint8_t)i2c_stm->i2c_reg->DR;

                        /* Wait for transmission to stop. */
                        I2C_STM_TIMED(i2c_stm->i2c_reg->CR1 & I2C_CR1_STOP);
                    }

                    if (status == SUCCESS)
                    {
                        /* Clear the position flag. */
                        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_POS);

                        /* Enable ACK to receive more data. */
                        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_ACK);
                    }

                    break;

                /* Three or more bytes to receive. */
                default:

                    /* Clear the address. */
                    i2c_stm->i2c_reg->SR1;
                    i2c_stm->i2c_reg->SR2;

                    /* Receive all the data. */
                    for (i = 0; ((status == SUCCESS) && (i < (message->length - 3))); i++)
                    {
                        /* Wait for a byte to be received. */
                        I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_RXNE));

                        /* Read a byte. */
                        message->buffer[i] = (uint8_t)i2c_stm->i2c_reg->DR;
                    }

                    if (status == SUCCESS)
                    {
                        /* Wait for byte transmission to finish. */
                        I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_BTF));
                    }

                    if (status == SUCCESS)
                    {
                        /* Clear the ACK. */
                        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

                        /* Disable global interrupts. */
                        DISABLE_INTERRUPTS();

                        /* Receive a byte. */
                        message->buffer[i++] = (uint8_t)i2c_stm->i2c_reg->DR;

                        /* Trigger stop. */
                        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

                        /* Restore old interrupt level. */
                        SET_INTERRUPT_LEVEL(interrupt_level);

                        /* Read next byte. */
                        message->buffer[i++] = (uint8_t)i2c_stm->i2c_reg->DR;

                        /* Wait for next byte to be received. */
                        I2C_STM_TIMED(!(i2c_stm->i2c_reg->SR1 & I2C_SR1_RXNE));
                    }

                    if (status == SUCCESS)
                    {
                        /* Read the last byte. */
                        message->buffer[i++] = (uint8_t)i2c_stm->i2c_reg->DR;

                        /* Wait for stop to happen. */
                        I2C_STM_TIMED(i2c_stm->i2c_reg->CR1 & I2C_CR1_STOP);
                    }

                    if (status == SUCCESS)
                    {
                        /* Clear the position flag. */
                        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_POS);

                        /* Enable ACK to receive more data. */
                        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_ACK);
                    }

                    break;
                }

                break;
            }
        }
#endif /* STM_I2C_INT_MODE */
    }

    if (status != SUCCESS)
    {
        /* Reinitialize this device to recover from error state. */
        i2c_stm32f103_init(device);
    }

    /* Return status to the caller. */
    return (status);

} /* i2c_stm32f103_message */

#ifdef STM_I2C_INT_MODE
/*
 * i2c1_event_interrupt
 * This function handles the I2C event interrupt.
 */
ISR_FUN i2c1_event_interrupt(void)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)i2c1_data->data;

    ISR_ENTER();

    /* If we have a start bit condition. */
    if (i2c_stm->i2c_reg->SR1 & I2C_SR1_SB)
    {
        /* Send the address byte. */
        i2c_stm->i2c_reg->DR = (uint16_t)((i2c1_data->address << 1) | (i2c_stm->msg->flags == I2C_MSG_READ));
    }

    /* If we have sent the address. */
    else if (i2c_stm->i2c_reg->SR1 & I2C_SR1_ADDR)
    {
        /* Process the message request. */
        switch (i2c_stm->msg->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
        {

        /* If we are reading from I2C bus. */
        case I2C_MSG_READ:

            /* Process according to the number of bytes to be sent. */
            switch (i2c_stm->msg->length)
            {

            /* If we are transferring a single byte. */
            case 1:

                /* Clear the ACK bit. */
                i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

                /* Clear the address. */
                i2c_stm->i2c_reg->SR1;
                i2c_stm->i2c_reg->SR2;

                break;

            /* If we are transferring two byte. */
            case 2:

                /* Set position flag. */
                i2c_stm->i2c_reg->CR1 |= (I2C_CR1_POS);

                /* Disable ACK. */
                i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

                /* Clear the address. */
                i2c_stm->i2c_reg->SR1;
                i2c_stm->i2c_reg->SR2;

                break;

            /* If we are transferring more than one byte. */
            default:

                /* Clear the address. */
                i2c_stm->i2c_reg->SR1;
                i2c_stm->i2c_reg->SR2;
            }
            break;

        /* If we are writing on I2C bus. */
        case I2C_MSG_WRITE:

            /* Clear the address. */
            i2c_stm->i2c_reg->SR1;
            i2c_stm->i2c_reg->SR2;

            break;
        }
    }

    /* If we just transfered a byte. */
    else if (i2c_stm->i2c_reg->SR1 & I2C_SR1_BTF)
    {
        /* Process the message request. */
        switch (i2c_stm->msg->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
        {

        /* If we are reading from I2C bus. */
        case I2C_MSG_READ:

            /* Process the number of bytes we need to read. */
            switch (i2c_stm->msg->length - i2c_stm->bytes_transfered)
            {

            /* Nothing to be read. */
            case 0:

                /* Stop right after this byte. */
                i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

                /* Disable buffer interrupt. */
                i2c_stm->i2c_reg->CR2 &= (uint16_t)~(I2C_CR2_ITBUFEN);

                /* Read this byte. */
                i2c_stm->i2c_reg->DR;

                /* Set the ping flag for I2C condition. */
                i2c_stm->condition.flags |= CONDITION_PING;

                /* This should never happen, so return an error. */
                i2c_stm->flags = I2C_STM32_ERROR;

                /* Resume any tasks waiting for I2C condition. */
                resume_condition(&i2c_stm->condition, NULL, TRUE);

                break;

            /* If we need to read a byte. */
            case 1:

                /* We will stop right after this byte. */
                i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

                /* Disable buffer interrupt. */
                i2c_stm->i2c_reg->CR2 &= (uint16_t)~(I2C_CR2_ITBUFEN);

                /* Read this byte. */
                i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
                i2c_stm->bytes_transfered ++;

                /* Set the ping flag for I2C condition. */
                i2c_stm->condition.flags |= CONDITION_PING;

                /* Resume any tasks waiting for I2C condition. */
                resume_condition(&i2c_stm->condition, NULL, TRUE);

                break;

            /* If we need to read two bytes. */
            case 2:
                /* Trigger stop. */
                i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

                /* Receive first byte. */
                i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
                i2c_stm->bytes_transfered ++;

                /* Receive second byte. */
                i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
                i2c_stm->bytes_transfered ++;

                /* Set the ping flag for I2C condition. */
                i2c_stm->condition.flags |= CONDITION_PING;

                /* Resume any tasks waiting for I2C condition. */
                resume_condition(&i2c_stm->condition, NULL, TRUE);

                break;

            /* If we need to read three bytes. */
            case 3:

                /* Clear the ACK. */
                i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_ACK);

                /* Receive a byte. */
                i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
                i2c_stm->bytes_transfered ++;

                break;

            /* If we need to read more than three bytes. */
            default:

                /* Receive a byte. */
                i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
                i2c_stm->bytes_transfered ++;

                break;
            }

            break;

        /* If we are writing on I2C bus. */
        case I2C_MSG_WRITE:

            /* Send the stop bit. */
            i2c_stm->i2c_reg->CR1 |= I2C_CR1_STOP;

            /* Receive to clear BTF. */
            i2c_stm->i2c_reg->DR;

            /* Set the ping flag for I2C condition. */
            i2c_stm->condition.flags |= CONDITION_PING;

            /* Resume any tasks waiting for I2C condition. */
            resume_condition(&i2c_stm->condition, NULL, TRUE);

            break;
        }
    }

    /* If transfer register is empty and we are writing data. */
    else if ((i2c_stm->i2c_reg->SR1 & I2C_SR1_TXE) && ((i2c_stm->msg->flags & (I2C_MSG_WRITE | I2C_MSG_READ)) == I2C_MSG_WRITE))
    {
        /* If we are about to send the last byte. */
        if ((i2c_stm->msg->length - i2c_stm->bytes_transfered) == 1)
        {
            /* Disable buffer interrupt. */
            i2c_stm->i2c_reg->CR2 &= (uint16_t)~(I2C_CR2_ITBUFEN);
        }

        /* Send this byte. */
        i2c_stm->i2c_reg->DR = (uint8_t)i2c_stm->msg->buffer[i2c_stm->bytes_transfered];
        i2c_stm->bytes_transfered ++;
    }

    /* If receive register is not empty and we are reading data. */
    else if ((i2c_stm->i2c_reg->SR1 & I2C_SR1_RXNE) && ((i2c_stm->msg->flags & (I2C_MSG_WRITE | I2C_MSG_READ)) == I2C_MSG_READ))
    {
        /* Process the number of bytes we need to read. */
        switch (i2c_stm->msg->length - i2c_stm->bytes_transfered)
        {
        /* Nothing to be read. */
        case 0:

            /* Clear the read flag. */
            i2c_stm->i2c_reg->DR;

            break;

        /* If we need to read more one byte. */
        case 1:

            /* We will stop right after this byte. */
            i2c_stm->i2c_reg->CR1 |= (I2C_CR1_STOP);

            /* Read this byte. */
            i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
            i2c_stm->bytes_transfered ++;

            /* Set the ping flag for I2C condition. */
            i2c_stm->condition.flags |= CONDITION_PING;

            /* Resume any tasks waiting for I2C condition. */
            resume_condition(&i2c_stm->condition, NULL, TRUE);

            break;

        /* If we need to read two or three bytes. */
        case 2:
        case 3:

            /* Disable buffer interrupt. */
            i2c_stm->i2c_reg->CR2 &= (uint16_t)~(I2C_CR2_ITBUFEN);

            break;

        /* If we need to read more than three bytes. */
        default:

            /* Receive a byte. */
            i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->DR;
            i2c_stm->bytes_transfered ++;

            break;
        }
    }

    /* If stop flag is set. */
    else if (i2c_stm->i2c_reg->SR1 & I2C_SR1_STOPF)
    {
        /* ACK data in next transactions. */
        i2c_stm->i2c_reg->CR1 |= (I2C_CR1_ACK);

        /* This should never happen, so return an error. */
        i2c_stm->flags = I2C_STM32_ERROR;

        /* Set the ping flag for I2C condition. */
        i2c_stm->condition.flags |= CONDITION_PING;

        /* Resume any tasks waiting for I2C condition. */
        resume_condition(&i2c_stm->condition, NULL, TRUE);
    }

    ISR_EXIT();

} /* i2c1_event_interrupt */

/*
 * i2c1_error_interrupt
 * This function handles the I2C error interrupt.
 */
ISR_FUN i2c1_error_interrupt(void)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)i2c1_data->data;
    ISR_ENTER();

    /* If acknowledgment failed. */
    if (i2c_stm->i2c_reg->SR1 & I2C_SR1_AF)
    {
        /* Clear the ACK failure. */
        i2c_stm->i2c_reg->SR1 &= (uint16_t)~(I2C_SR1_AF);

        /* Remote sent a NAK. */
        i2c_stm->flags = I2C_STM32_NACK;
    }
    else
    {
        /* A bus error was detected. */
        if (i2c_stm->i2c_reg->SR1 & I2C_SR1_BERR)
        {
            /* Clear bus error. */
            i2c_stm->i2c_reg->SR1 &= (uint16_t)~(I2C_SR1_BERR);
        }

        /* A arbitration lost error was detected. */
        if (i2c_stm->i2c_reg->SR1 & I2C_SR1_ARLO)
        {
            /* Clear arbitration lost error. */
            i2c_stm->i2c_reg->SR1 &= (uint16_t)~(I2C_SR1_ARLO);
        }

        /* Return error to the caller. */
        i2c_stm->flags = I2C_STM32_ERROR;
    }

    /* Set the ping flag for I2C condition. */
    i2c_stm->condition.flags |= CONDITION_PING;

    /* Resume any tasks waiting for I2C condition. */
    resume_condition(&i2c_stm->condition, NULL, TRUE);

    ISR_EXIT();

} /* i2c1_error_interrupt */

/*
 * i2c1_stm32f103_enable_interrupt.
 * This function will enable interrupts for the given I2C device.
 */
static void i2c1_stm32f103_enable_interrupt(void *data)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)((I2C_DEVICE *)data)->data;

    /* Process I2C device number. */
    switch (i2c_stm->device_num)
    {

    /* This is I2C1 device. */
    case 1:
        /* Enable the I2C1 event and error channels. */
        NVIC->ISER[I2C1_EV_IRQn >> 0x05] = (uint32_t)0x01 << (I2C1_EV_IRQn & (uint8_t)0x1F);
        NVIC->ISER[I2C1_ER_IRQn >> 0x05] = (uint32_t)0x01 << (I2C1_ER_IRQn & (uint8_t)0x1F);

        break;

    /* This is I2C2 device. */
    case 2:
        /* Enable the I2C2 event and error channels. */
        NVIC->ISER[I2C2_EV_IRQn >> 0x05] = (uint32_t)0x01 << (I2C2_EV_IRQn & (uint8_t)0x1F);
        NVIC->ISER[I2C2_ER_IRQn >> 0x05] = (uint32_t)0x01 << (I2C2_ER_IRQn & (uint8_t)0x1F);

        break;

    /* Unknown device. */
    default:

        /* Nothing to do here. */
        break;
    }

} /* i2c1_stm32f103_enable_interrupt */

/*
 * i2c2_stm32f103_disable_interrupt.
 * This function will disable interrupts for the given I2C device.
 */
static void i2c2_stm32f103_disable_interrupt(void *data)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)((I2C_DEVICE *)data)->data;

    /* Process I2C device number. */
    switch (i2c_stm->device_num)
    {

    /* This is I2C1 device. */
    case 1:
        /* Disable the I2C1 event and error channels. */
        NVIC->ICER[I2C1_EV_IRQn >> 0x05] = (uint32_t)0x01 << (I2C1_EV_IRQn & (uint8_t)0x1F);
        NVIC->ICER[I2C1_ER_IRQn >> 0x05] = (uint32_t)0x01 << (I2C1_ER_IRQn & (uint8_t)0x1F);

        break;

    /* This is I2C2 device. */
    case 2:
        /* Disable the I2C2 event and error channels. */
        NVIC->ICER[I2C2_EV_IRQn >> 0x05] = (uint32_t)0x01 << (I2C2_EV_IRQn & (uint8_t)0x1F);
        NVIC->ICER[I2C2_ER_IRQn >> 0x05] = (uint32_t)0x01 << (I2C2_ER_IRQn & (uint8_t)0x1F);

        break;

    /* Unknown device. */
    default:

        /* Nothing to do here. */
        break;
    }

} /* i2c2_stm32f103_disable_interrupt */
#endif /* STM_I2C_INT_MODE */
#endif /* CONFIG_I2C */
