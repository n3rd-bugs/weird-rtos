/*
 * enc28j60_stm32f407.c
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
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_stm32f407.h>
#include <string.h>

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;

/*
 * enc28j60_stm32f407_init
 * This function will initialize enc28j60 device for stm32f407 platform.
 */
void enc28j60_stm32f407_init()
{
    /* Clear the enc28j60 device structure. */
    memset(&enc28j60, 0, sizeof(ENC28J60));

    /* Initialize SPI device data. */
    enc28j60.spi.data.device_num = 1;

    /* Enable GPIO A clock. */
    RCC->AHB1ENR |= 0x00000001;

    /* Configure GPIO mode input for GPIOA.2. */
    GPIOA->MODER &= (uint32_t)(~(GPIO_MODER_MODER0 << (2 * 2)));

    /* Configure output type (PP). */
    GPIOA->OTYPER &= (uint32_t)(~(GPIO_OTYPER_OT_0 << (2 * 2)));

    /* Enable pull-up on GPIOA.2. */
    GPIOA->PUPDR &= (uint32_t)(~(GPIO_PUPDR_PUPDR0 << (2 * 2)));
    GPIOA->PUPDR |= (0x01 << (2 * 2));

    /* Configure GPIO speed (100MHz). */
    GPIOA->OSPEEDR &= (uint32_t)(~(GPIO_OSPEEDER_OSPEEDR0 << (2 * 2)));
    GPIOA->OSPEEDR |= (0x03 << (2 * 2));

    /* Set EXTI line for processing interrupts on GPIOA.2.s */
    SYSCFG->EXTICR[(0x02 >> 0x02)] &= (uint32_t)(~(0x0F << (0x04 * (0x02 & 0x03))));
    SYSCFG->EXTICR[(0x02 >> 0x02)] |= (0x00 << (0x04 * (0x02 & 0x03)));

    /* Clear EXTI line configuration. */
    EXTI->IMR &= (uint32_t)~(0x04);
    EXTI->EMR &= (uint32_t)~(0x04);
    EXTI->RTSR &= (uint32_t)~(0x04);
    EXTI->FTSR &= (uint32_t)~(0x04);

    /* Enable interrupt mode. */
    *(uint32_t *) (EXTI_BASE + 0x00) |= (0x04);

    /* Enable interrupt for falling edge. */
    *(uint32_t *) (EXTI_BASE + 0x0C) |= (0x04);

    /* Set EXT2 IRQ channel priority. */
    NVIC->IP[EXTI2_IRQn] = 2;

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_stm32f407_init */

/*
 * enc28j60_stm32f407_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_stm32f407_handle_interrupt()
{
    /* Resume tasks waiting for this networking device. */

    /* Disable interrupt until we process it. */
    enc28j60_stm32f407_disable_interrupt(&enc28j60);

} /* enc28j60_stm32f407_handle_interrupt */

/*
 * enc28j60_stm32f407_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f407_enable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Enable the EXT2 IRQ channel. */
    NVIC->ISER[EXTI2_IRQn >> 0x05] = (uint32_t)0x01 << (EXTI2_IRQn & (uint8_t)0x1F);

} /* enc28j60_stm32f407_enable_interrupt */

/*
 * enc28j60_stm32f407_disable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  disabled.
 * This function will disable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f407_disable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Disable the EXT2 IRQ channel. */
    NVIC->ICER[EXTI2_IRQn >> 0x05] = (uint32_t)0x01 << (EXTI2_IRQn & (uint8_t)0x1F);

} /* enc28j60_stm32f407_disable_interrupt */

#endif /* ETHERNET_ENC28J60 */
