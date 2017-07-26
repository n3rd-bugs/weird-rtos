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
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#include <kernel.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_stm32f407.h>
#include <string.h>
#include <spi_stm32f407.h>
#include <net_csum.h>

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;
static STM32F407_SPI enc28j60_spi;

/*
 * enc28j60_stm32f407_init
 * This function will initialize enc28j60 device for stm32f407 platform.
 */
void enc28j60_stm32f407_init()
{
    /* Clear the enc28j60 device structures. */
    memset(&enc28j60, 0, sizeof(ENC28J60));
    memset(&enc28j60_spi, 0, sizeof(STM32F407_SPI));

    /* Initialize SPI device data. */
    enc28j60_spi.device_num = 1;
    enc28j60.spi.init = &spi_stm32f407_init;
    enc28j60.spi.slave_select = &spi_stm32f407_slave_select;
    enc28j60.spi.slave_unselect = &spi_stm32f407_slave_unselect;
    enc28j60.spi.msg = &spi_stm32f407_message;
    enc28j60.spi.data = &enc28j60_spi;

    /* Enable GPIO A clock. */
    RCC->AHB1ENR |= 0x00000001;

    /* Configure GPIO mode input for GPIOA.2 and output for GPIOA.3. */
    GPIOA->MODER &= ~((GPIO_MODER_MODER0 << (2 * 2)) | (GPIO_MODER_MODER0 << (3 * 2)));
    GPIOA->MODER |= (0x01 << (3 * 2));

    /* Configure output type (PP). */
    GPIOA->OTYPER &= (uint32_t)(~((GPIO_OTYPER_OT_0 << (2 * 2)) | (GPIO_OTYPER_OT_0 << (3 * 2))));

    /* Enable pull-up on GPIOA.2 and GPIOA.3. */
    GPIOA->PUPDR &= (uint32_t)(~(((GPIO_PUPDR_PUPDR0 << (2 * 2)) | (GPIO_PUPDR_PUPDR0 << (3 * 2)))));
    GPIOA->PUPDR |= (0x01 << (2 * 2));

    /* Configure GPIO speed (100MHz). */
    GPIOA->OSPEEDR &= (uint32_t)(~((GPIO_OSPEEDER_OSPEEDR0 << (2 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (3 * 2))));
    GPIOA->OSPEEDR |= ((0x03 << (2 * 2)) | (0x03 << (3 * 2)));

#if (ENC28J60_INT_POLL == FALSE)
    /* Set EXTI line for processing interrupts on GPIOA.2. */
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
#endif

    /* Initialize device APIs. */
#if (ENC28J60_INT_POLL == FALSE)
    enc28j60.enable_interrupts = &enc28j60_stm32f407_enable_interrupt;
    enc28j60.disable_interrupts = &enc28j60_stm32f407_disable_interrupt;
#endif
    enc28j60.interrupt_pin = &enc28j60_stm32f407_interrupt_pin;
    enc28j60.reset = &enc28j60_stm32f407_reset;
    enc28j60.get_mac = &enc28j60_stm32f407_get_mac;

    /* Initialize name for this device. */
    enc28j60.ethernet_device.fs.name = "\\ethernet\\enc28j60";

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_stm32f407_init */

#if (ENC28J60_INT_POLL == FALSE)
/*
 * enc28j60_stm32f407_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_stm32f407_handle_interrupt()
{
    /* Disable interrupt until we process it. */
    enc28j60.flags &= (uint8_t)~(ENC28J60_INT_ENABLE);
    enc28j60_stm32f407_disable_interrupt(&enc28j60);

    /* Handle interrupt for this device. */
    ethernet_interrupt(&enc28j60.ethernet_device);

} /* enc28j60_stm32f407_handle_interrupt */

/*
 * enc28j60_stm32f407_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f407_enable_interrupt(ENC28J60 *device)
{
    /* If we need to enable interrupts for this device. */
    if (device->flags & ENC28J60_INT_ENABLE)
    {
        /* Enable the EXT2 IRQ channel. */
        NVIC->ISER[EXTI2_IRQn >> 0x05] = (uint32_t)0x01 << (EXTI2_IRQn & (uint8_t)0x1F);
    }

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
#endif

/*
 * enc28j60_stm32f407_interrupt_pin
 * device: ENC28J60 device instance for which we need to query the status of
 * interrupt pin.
 * This function will return the status of interrupt pin.
 */
uint8_t enc28j60_stm32f407_interrupt_pin(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Return if interrupt pin is high. */
    return ((GPIOA->IDR & (1 << 2)) != FALSE);

} /* enc28j60_stm32f407_interrupt_pin */

/*
 * enc28j60_stm32f407_reset
 * device: ENC28J60 device needed to be reset.
 * This function will reset the target enc28j60 device.
 */
void enc28j60_stm32f407_reset(ENC28J60 *device)
{
    FD *fd = (FD)device;

    /* Clear the RST, i.e. GPIOA.3. */
    GPIOA->BSRR |= (1 << (3 + 16));

    /* Release lock for this device. */
    fd_release_lock(fd);

    /* Sleep to wait for target to actually reset. */
    sleep_fms(ENC28J60_STM32F407_RESET_DELAY);

    /* Acquire lock for this device. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Set the RST, i.e. GPIOA.3. */
    GPIOA->BSRR |= (1 << 3);

} /* enc28j60_stm32f407_reset */

/*
 * enc28j60_stm32f407_get_mac
 * @device: Ethernet device instance for which MAC address is needed to
 *  be assigned.
 * @return: Returns the start of random MAC address generated.
 * This function will generate a MAC address using the device serial.
 */
uint8_t *enc28j60_stm32f407_get_mac(ETH_DEVICE *device)
{
    /* Push ENC28j60 OUI bytes. */
    device->mac[0] = ENC28J60_OUI_B0;
    device->mac[1] = ENC28J60_OUI_B1;
    device->mac[2] = ENC28J60_OUI_B2;

    /* Assign remaining MAC address using the device serial. */
    device->mac[3] = (uint8_t)NET_CSUM_BYTE(NET_CSUM_BYTE(NET_CSUM_BYTE(STM32_UUID[0], STM32_UUID[1]), STM32_UUID[2]), STM32_UUID[3]);
    device->mac[4] = (uint8_t)NET_CSUM_BYTE(NET_CSUM_BYTE(NET_CSUM_BYTE(STM32_UUID[4], STM32_UUID[5]), STM32_UUID[6]), STM32_UUID[7]);
    device->mac[5] = (uint8_t)NET_CSUM_BYTE(NET_CSUM_BYTE(NET_CSUM_BYTE(STM32_UUID[8], STM32_UUID[9]), STM32_UUID[10]), STM32_UUID[11]);

    /* Return the generated MAC address. */
    return (device->mac);

} /* enc28j60_stm32f407_get_mac */

#endif /* ETHERNET_ENC28J60 */
