/*
 * i2c_stm32f030.c
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

#ifdef IO_I2C
#include <i2c_stm32f030.h>

#ifdef STM_I2C_INT_MODE
/* I2C device data. */
static I2C_DEVICE *i2c1_data;

/* Internal function prototypes. */
static void i2c1_stm32f030_enable_interrupt(void *);
static void i2c2_stm32f030_disable_interrupt(void *);
#endif /* STM_I2C_INT_MODE */

/*
 * i2c_stm32f030_init
 * @device: I2C device needed to be initialized.
 * This function will initialize a bit-bang I2C device instance.
 */
void i2c_stm32f030_init(I2C_DEVICE *device)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)device->data;

    /* Pick the required I2C device. */
    switch (i2c_stm->device_num)
    {

    /* This is I2C1. */
    case 1:

        /* Enable clock for GPIOA. */
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

        /* Set alternate function for PA9 (SCL) and PA10 (SDA). */
        GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
        GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

        /* Select output mode for PA9 and PA10. */
        GPIOA->OTYPER |= GPIO_OTYPER_OT_9 | GPIO_OTYPER_OT_10;

        /* Select high speed for PA9 and PA10 */
        GPIOA->OSPEEDR &= (uint32_t)~(GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10);
        GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR9_1 | GPIO_OSPEEDER_OSPEEDR9_0) |
                          (GPIO_OSPEEDER_OSPEEDR10_1 | GPIO_OSPEEDER_OSPEEDR10_0);

        /* Select pull-up for the lines. */
        GPIOA->PUPDR &= (uint32_t)~(GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR10);
        GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR10_0;

        /* Select I2C1 AF for PA9 and PA10. */
        GPIOA->AFR[1] &= (uint32_t)~((0xF << ((9 % 8) << 2)) | (0xF << ((10 % 8) << 2)));
        GPIOA->AFR[1] |= (0x4 << ((9 % 8) << 2)) | (0x4 << ((10 % 8) << 2));

        /* Reset I2C1. */
        RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
        RCC->APB1RSTR &= (uint32_t)~RCC_APB1RSTR_I2C1RST;

        /* Select system clock for I2C. */
        RCC->CFGR3 |= RCC_CFGR3_I2C1SW;

        /* Enable clock for I2C1. */
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

        /* Pick the required device register. */
        i2c_stm->i2c_reg = I2C1;

#ifdef STM_I2C_INT_MODE
        /* We should not register same device again with different data. */
        ASSERT((i2c1_data != NULL) && (i2c1_data != device));

        /* Save data for I2C1. */
        i2c1_data = device;
#endif /* STM_I2C_INT_MODE */

        break;

    /* Unknown device number. */
    default:

        /* Nothing to do here. */
        break;
    }

    /* Disable I2C device. */
    i2c_stm->i2c_reg->CR1 &= (uint32_t)~(I2C_CR1_PE);

    /* Setup timing register. */
    i2c_stm->i2c_reg->TIMINGR = i2c_stm->timing;

    /* Enable own address. */
    i2c_stm->i2c_reg->OAR1 = I2C_OAR1_OA1EN;

    /* Enable 7-bit addressing. */
    i2c_stm->i2c_reg->CR2 &= (uint32_t)~(I2C_CR2_ADD10);

    /* Enable the auto end mode by default. */
    i2c_stm->i2c_reg->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

    /* Enable device. */
    i2c_stm->i2c_reg->CR1 |= I2C_CR1_PE;

    /* Test if device is in busy state. */
    if (i2c_stm->i2c_reg->ISR & I2C_ISR_BUSY)
    {
        /* Power down the device. */
        i2c_stm->i2c_reg->CR1 &= (uint32_t)~(I2C_CR1_PE);

        /* Process I2C pins according to the selected device. */
        switch (i2c_stm->device_num)
        {

        /* This is I2C1. */
        case 1:
            /* Set alternate function for PA9 (SCL) and PA10 (SDA). */
            GPIOA->MODER &= (uint32_t)~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
            GPIOA->MODER |= (GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_0);
            GPIOA->PUPDR &= (uint32_t)~(GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR10);

            /* Toggle both pins. */
            GPIOA->ODR |= ((1 << 9) | (1 << 10));
            GPIOA->ODR &= (uint16_t)~((1 << 9) | (1 << 10));

            /* Bring then high again. */
            GPIOA->ODR |= ((1 << 9) | (1 << 10));

            /* Configure back to alternate function. */
            GPIOA->MODER |= (GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_0);

            break;

        /* Unknown device number. */
        default:

            /* Nothing to do here. */
            break;
        }

        /* Software reset the device. */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_SWRST;
        i2c_stm->i2c_reg->CR1 &= (uint16_t)~(I2C_CR1_SWRST);

        /* Disable I2C device. */
        i2c_stm->i2c_reg->CR1 &= (uint32_t)~((uint32_t)I2C_CR1_PE);

        /* Setup timing register. */
        i2c_stm->i2c_reg->TIMINGR = i2c_stm->timing;

        /* Enable 7-bit addressing. */
        i2c_stm->i2c_reg->CR2 &= (uint32_t)~(I2C_CR2_ADD10);

        /* Enable device. */
        i2c_stm->i2c_reg->CR1 |= I2C_CR1_PE;
    }

#ifdef STM_I2C_INT_MODE
    /* Enable I2C interrupts. */
    i2c1_stm32f030_enable_interrupt(device);

    /* Initialize I2C condition data. */
    i2c_stm->condition.lock = &i2c2_stm32f030_disable_interrupt;
    i2c_stm->condition.unlock = &i2c1_stm32f030_enable_interrupt;
    i2c_stm->condition.data = device;

#endif /* STM_I2C_INT_MODE */

} /* i2c_stm32f030_init */

/*
 * i2c_stm32f030_message
 * @device: I2C device for which messages are needed to be processed.
 * @message: I2C message needed to be sent.
 * @return: Success will be returned if I2C message was successfully processed,
 *  I2C_TIMEOUT will be returned if we timeout in waiting for an event,
 *  I2C_IO_ERROR will be returned if a I2C bus error was detected,
 *  I2C_MSG_NACKED will be returned if a byte was NACKed.
 * This function will process a I2C message.
 */
int32_t i2c_stm32f030_message(I2C_DEVICE *device, I2C_MSG *message)
{
    int32_t status = SUCCESS;
    I2C_STM32 *i2c_stm = (I2C_STM32 *)device->data;
    int32_t i;
    int32_t remaining_size = message->length;
#ifdef STM_I2C_INT_MODE
    INT_LVL interrupt_level = GET_INTERRUPT_LEVEL();
    CONDITION *condition = &i2c_stm->condition;
    SUSPEND *suspend = &i2c_stm->suspend;
#else
    int32_t j;
#endif /* STM_I2C_INT_MODE */
    uint8_t this_size;

    /* Wait for I2C to get out of busy state. */
    I2C_STM_TIMED(i2c_stm->i2c_reg->ISR & I2C_ISR_BUSY);

    if (status == SUCCESS)
    {
#ifdef STM_I2C_INT_MODE
        /* Initialize I2C bus data. */
        i2c_stm->msg = message;
        i2c_stm->bytes_transfered = 0;
        i2c_stm->flags = 0;

        /* Transfer all the data. */
        for (i = 0; ((i < message->length) && (status == SUCCESS));)
        {
            /* Clear the CR2. */
            i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));

            /* If we have more than 255 byte to be sent. */
            if (remaining_size > 0xFF)
            {
                /* Send on the the 255 bytes. */
                this_size = 0xFF;

                /* Update the CR2 with slave address, number of bytes to be sent and enable reload mode. */
                i2c_stm->i2c_reg->CR2 |= (uint32_t)(((uint32_t)(device->address << 1) & I2C_CR2_SADD) | (((uint32_t)this_size << 16 ) & I2C_CR2_NBYTES) | (uint32_t)I2C_CR2_RELOAD);
                i2c_stm->do_continue = TRUE;
            }
            else
            {
                /* Send all the data. */
                this_size = (uint8_t)remaining_size;

                /* Update the CR2 with slave address, number of bytes to be sent and enable auto end mode. */
                i2c_stm->i2c_reg->CR2 |= (uint32_t)(((uint32_t)(device->address << 1) & I2C_CR2_SADD) | (((uint32_t)this_size << 16 ) & I2C_CR2_NBYTES) | (uint32_t)I2C_CR2_AUTOEND);
                i2c_stm->do_continue = FALSE;
            }

            /* Save the number of bytes to be transfered. */
            i2c_stm->this_transfer = (uint8_t)this_size;

            /* If this is a read request. */
            if ((message->flags & (I2C_MSG_WRITE | I2C_MSG_READ)) == I2C_MSG_READ)
            {
                /* This is a read request. */
                i2c_stm->i2c_reg->CR2 |= (I2C_CR2_RD_WRN);
            }

            /* Disable interrupts. */
            DISABLE_INTERRUPTS();

            /* Enable I2C interrupts. */
            i2c_stm->i2c_reg->CR1 |= (I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE | I2C_CR1_TCIE | I2C_CR1_ERRIE);

            /* Send I2C start. */
            i2c_stm->i2c_reg->CR2 |= I2C_CR2_START;

            /* Enable timeout for suspend. */
            i2c_stm->suspend.timeout_enabled = TRUE;
            i2c_stm->suspend.timeout = current_system_tick() + MS_TO_TICK(STM_I2C_INT_TIMEOUT);
            i2c_stm->suspend.priority = SUSPEND_MIN_PRIORITY;
            i2c_stm->suspend.status = SUCCESS;

            /* Suspend on the I2C interrupts to process this message. */
            status = suspend_condition(&condition, &suspend, NULL, FALSE);

            /* Disable buffer, event and error interrupts for this I2C. */
            i2c_stm->i2c_reg->CR1 &= (uint32_t)~(I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE | I2C_CR1_TCIE | I2C_CR1_ERRIE);

            /* Restore old interrupt level. */
            SET_INTERRUPT_LEVEL(interrupt_level);

            if (status == SUCCESS)
            {
                /* If a bus error was detected or not all of the data was transfered. */
                if ((i2c_stm->flags & I2C_STM32_ERROR) || (i2c_stm->this_transfer > 0))
                {
                    /* Return IO error to the caller. */
                    status = I2C_IO_ERROR;
                }

                /* If remote sent a NAK. */
                else if (i2c_stm->flags & I2C_STM32_NACK)
                {
                    /* Return NAK error to the caller. */
                    status = I2C_MSG_NACKED;
                }

                else
                {
                    /* We just transfered some data. */
                    i += this_size;
                    remaining_size -= this_size;
                }
            }

            /* If we timeout waiting for this transaction. */
            else if (status == CONDITION_TIMEOUT)
            {
                /* Return timeout error to the caller. */
                status = I2C_TIMEOUT;
            }
        }
#else
        if (status == SUCCESS)
        {
            /* Process the message request. */
            switch (message->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
            {

            /* If we are writing. */
            case I2C_MSG_WRITE:

                /* Transfer all the bytes in this message. */
                for (i = 0; ((i < message->length) && (status == SUCCESS)); )
                {
                    /* Clear the CR2. */
                    i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));

                    /* If we have more than 255 byte to be sent. */
                    if (remaining_size > 0xFF)
                    {
                        /* Send on the the 255 bytes. */
                        this_size = 0xFF;

                        /* Update the CR2 with slave address, number of bytes to be sent and enable reload mode. */
                        i2c_stm->i2c_reg->CR2 |= (uint32_t)(((uint32_t)(device->address << 1) & I2C_CR2_SADD) | (((uint32_t)this_size << 16 ) & I2C_CR2_NBYTES) | (uint32_t)I2C_CR2_RELOAD);

                    }
                    else
                    {
                        /* Send all the data. */
                        this_size = (uint8_t)remaining_size;

                        /* Update the CR2 with slave address, number of bytes to be sent and enable auto end mode. */
                        i2c_stm->i2c_reg->CR2 |= (uint32_t)(((uint32_t)(device->address << 1) & I2C_CR2_SADD) | (((uint32_t)this_size << 16 ) & I2C_CR2_NBYTES) | (uint32_t)I2C_CR2_AUTOEND);
                    }

                    /* Send I2C start. */
                    i2c_stm->i2c_reg->CR2 |= I2C_CR2_START;

                    for (j = 0; ((i < message->length) && (j < this_size ) && (status == SUCCESS)); j++, i++)
                    {
                        /* Wait for the transmission interrupt. */
                        I2C_STM_TIMED(!(i2c_stm->i2c_reg->ISR & I2C_ISR_TXIS));

                        /* Send this byte. */
                        i2c_stm->i2c_reg->TXDR = (uint8_t)message->buffer[i];
                    }

                    /* Wait for the TCR to clear. */
                    I2C_STM_TIMED((i2c_stm->i2c_reg->ISR & I2C_ISR_TCR));

                    if (status == SUCCESS)
                    {
                        /* We just transfered some data. */
                        remaining_size -= this_size;
                    }
                }

                if (status == SUCCESS)
                {
                    /* Wait for the STOP condition. */
                    I2C_STM_TIMED(!(i2c_stm->i2c_reg->ISR & I2C_ISR_STOPF));

                    /* Clear the stop flag. */
                    i2c_stm->i2c_reg->ICR = I2C_ICR_STOPCF;

                    /* Clear CR2. */
                    i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
                }

                break;

            /* If we are reading. */
            case I2C_MSG_READ:

                /* Read all the requested bytes. */
                for (i = 0; ((i < message->length) && (status == SUCCESS)); )
                {
                    /* Clear the CR2. */
                    i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));

                    /* If we have more than 255 byte to be sent. */
                    if (remaining_size > 0xFF)
                    {
                        /* Send on the the 255 bytes. */
                        this_size = 0xFF;

                        /* Update the CR2 with slave address, number of bytes to be sent and enable reload mode. */
                        i2c_stm->i2c_reg->CR2 |= (uint32_t)(((uint32_t)(device->address << 1) & I2C_CR2_SADD) | (((uint32_t)this_size << 16 ) & I2C_CR2_NBYTES) | (uint32_t)I2C_CR2_RELOAD);

                    }
                    else
                    {
                        /* Send all the data. */
                        this_size = (uint8_t)remaining_size;

                        /* Update the CR2 with slave address, number of bytes to be sent and enable auto end mode. */
                        i2c_stm->i2c_reg->CR2 |= (uint32_t)(((uint32_t)(device->address << 1) & I2C_CR2_SADD) | (((uint32_t)this_size << 16 ) & I2C_CR2_NBYTES) | (uint32_t)I2C_CR2_AUTOEND);
                    }

                    /* This is a read request. */
                    i2c_stm->i2c_reg->CR2 |= (I2C_CR2_RD_WRN);

                    /* Generate the start bit. */
                    i2c_stm->i2c_reg->CR2 |= I2C_CR2_START;

                    /* Read all the bytes. */
                    for (j = 0; ((i < message->length) && (j < this_size ) && (status == SUCCESS)); j++, i++)
                    {
                        /* Wait for data to become available. */
                        I2C_STM_TIMED(!(i2c_stm->i2c_reg->ISR & I2C_ISR_RXNE));

                        /* Read a byte. */
                        message->buffer[i] = (uint8_t)i2c_stm->i2c_reg->RXDR;
                    }

                    /* Wait for the TCR. */
                    I2C_STM_TIMED((i2c_stm->i2c_reg->ISR & I2C_ISR_TCR));

                    if (status == SUCCESS)
                    {
                        /* We just transfered some data. */
                        remaining_size -= this_size;
                    }
                }

                if (status == SUCCESS)
                {
                    /* Wait for the STOP condition. */
                    I2C_STM_TIMED(!(i2c_stm->i2c_reg->ISR & I2C_ISR_STOPF));

                    /* Clear the stop flag. */
                    i2c_stm->i2c_reg->ICR = I2C_ICR_STOPCF;

                    /* Clear CR2. */
                    i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
                }

                break;
            }
        }
#endif /* STM_I2C_INT_MODE */
    }

    if (status != SUCCESS)
    {
        /* Reinitialize this device to recover from error state. */
        i2c_stm32f030_init(device);
    }

    /* Return status to the caller. */
    return (status);

} /* i2c_stm32f030_message */

#ifdef STM_I2C_INT_MODE
/*
 * i2c1_event_interrupt
 * This function handles the I2C event interrupt.
 */
ISR_FUN i2c1_event_interrupt(void)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)i2c1_data->data;

    ISR_ENTER();

    /* If we just received a NACK. */
    if (i2c_stm->i2c_reg->ISR & I2C_ISR_NACKF)
    {
        /* Clear the MACK flag. */
        i2c_stm->i2c_reg->ICR = I2C_ICR_NACKCF;

        /* Set the ping flag for I2C condition. */
        i2c_stm->condition.flags |= CONDITION_PING;
        i2c_stm->flags = I2C_STM32_NACK;

        /* Resume any tasks waiting for I2C condition. */
        resume_condition(&i2c_stm->condition, NULL, TRUE);
    }

    /* If we just received an error. */
    else if ((i2c_stm->i2c_reg->ISR & I2C_ISR_BERR) || (i2c_stm->i2c_reg->ISR & I2C_ISR_ARLO) || (i2c_stm->i2c_reg->ISR & I2C_ISR_OVR) || (i2c_stm->i2c_reg->ISR & I2C_ISR_PECERR) || (i2c_stm->i2c_reg->ISR & I2C_ISR_TIMEOUT))
    {
        /* Clear the errors. */
        i2c_stm->i2c_reg->ICR = I2C_ICR_BERRCF;
        i2c_stm->i2c_reg->ICR = I2C_ICR_ARLOCF;
        i2c_stm->i2c_reg->ICR = I2C_ICR_OVRCF;
        i2c_stm->i2c_reg->ICR = I2C_ICR_PECCF;
        i2c_stm->i2c_reg->ICR = I2C_ICR_TIMOUTCF;

        /* Set the error flag. */
        i2c_stm->flags = I2C_STM32_ERROR;

        /* Clear CR2. */
        i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));

        /* Set the ping flag for I2C condition. */
        i2c_stm->condition.flags |= CONDITION_PING;

        /* Resume any tasks waiting for I2C condition. */
        resume_condition(&i2c_stm->condition, NULL, TRUE);
    }

    /* If we just received a stop. */
    else if (i2c_stm->i2c_reg->ISR & I2C_ISR_STOPF)
    {
        /* Clear the stop flag. */
        i2c_stm->i2c_reg->ICR = I2C_ICR_STOPCF;

        /* Clear CR2. */
        i2c_stm->i2c_reg->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));

        /* Set the ping flag for I2C condition. */
        i2c_stm->condition.flags |= CONDITION_PING;

        /* Resume any tasks waiting for I2C condition. */
        resume_condition(&i2c_stm->condition, NULL, TRUE);
    }
    else
    {
        /* If we have more data to be sent. */
        if (i2c_stm->this_transfer > 0)
        {
            /* Process the message request. */
            switch (i2c_stm->msg->flags & (I2C_MSG_WRITE | I2C_MSG_READ))
            {
            /* If we are writing on I2C bus. */
            case I2C_MSG_WRITE:

                /* If we can send more data. */
                if (i2c_stm->i2c_reg->ISR & I2C_ISR_TXIS)
                {
                    /* Send this byte. */
                    i2c_stm->i2c_reg->TXDR = (uint8_t)i2c_stm->msg->buffer[i2c_stm->bytes_transfered];

                    /* Just send a byte. */
                    i2c_stm->bytes_transfered ++;
                    i2c_stm->this_transfer--;
                }

                break;

            /* If we are reading from I2C bus. */
            case I2C_MSG_READ:

                /* If we can send more data. */
                if (i2c_stm->i2c_reg->ISR & I2C_ISR_RXNE)
                {
                    /* Read a byte. */
                    i2c_stm->msg->buffer[i2c_stm->bytes_transfered] = (uint8_t)i2c_stm->i2c_reg->RXDR;

                    /* Just read a byte. */
                    i2c_stm->bytes_transfered ++;
                    i2c_stm->this_transfer--;
                }

                break;
            }
        }

        /* If we just transfer all the bytes needed to be sent and need to continue. */
        if ((i2c_stm->this_transfer == 0) && (i2c_stm->do_continue == TRUE))
        {
            /* Set the ping flag for I2C condition. */
            i2c_stm->condition.flags |= CONDITION_PING;

            /* Resume any tasks waiting for I2C condition. */
            resume_condition(&i2c_stm->condition, NULL, TRUE);
        }
    }

    ISR_EXIT();

} /* i2c1_event_interrupt */

/*
 * i2c1_stm32f030_enable_interrupt.
 * This function will enable interrupts for the given I2C device.
 */
static void i2c1_stm32f030_enable_interrupt(void *data)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)((I2C_DEVICE *)data)->data;

    /* Process I2C device number. */
    switch (i2c_stm->device_num)
    {

    /* This is I2C1 device. */
    case 1:
        /* Enable the I2C1 event channels. */
        NVIC->ISER[I2C1_IRQn >> 0x5] = (uint32_t)0x1 << (I2C1_IRQn & (uint8_t)0x1F);

        break;

    /* Unknown device. */
    default:

        /* Nothing to do here. */
        break;
    }

} /* i2c1_stm32f030_enable_interrupt */

/*
 * i2c2_stm32f030_disable_interrupt.
 * This function will disable interrupts for the given I2C device.
 */
static void i2c2_stm32f030_disable_interrupt(void *data)
{
    I2C_STM32 *i2c_stm = (I2C_STM32 *)((I2C_DEVICE *)data)->data;

    /* Process I2C device number. */
    switch (i2c_stm->device_num)
    {

    /* This is I2C1 device. */
    case 1:
        /* Disable the I2C1 event channels. */
        NVIC->ICER[I2C1_IRQn >> 0x5] = (uint32_t)0x1 << (I2C1_IRQn & (uint8_t)0x1F);

        break;

    /* Unknown device. */
    default:

        /* Nothing to do here. */
        break;
    }

} /* i2c2_stm32f030_disable_interrupt */
#endif /* STM_I2C_INT_MODE */
#endif /* IO_I2C */
