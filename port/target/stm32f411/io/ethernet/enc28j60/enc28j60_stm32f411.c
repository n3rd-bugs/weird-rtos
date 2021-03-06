/*
 * enc28j60_stm32f411.c
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
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_stm32f411.h>
#include <string.h>
#include <spi_stm32f411.h>
#include <net_csum.h>

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;
static STM32F411_SPI enc28j60_spi;

/*
 * enc28j60_stm32f411_init
 * This function will initialize enc28j60 device for stm32f411 platform.
 */
void enc28j60_stm32f411_init(void)
{
    /* Clear the enc28j60 device structures. */
    memset(&enc28j60, 0, sizeof(ENC28J60));
    memset(&enc28j60_spi, 0, sizeof(STM32F411_SPI));

    /* Initialize SPI device data. */
    enc28j60_spi.device_num = 1;
    enc28j60.spi.init = &spi_stm32f411_init;
    enc28j60.spi.slave_select = &spi_stm32f411_slave_select;
    enc28j60.spi.slave_unselect = &spi_stm32f411_slave_unselect;
    enc28j60.spi.msg = &spi_stm32f411_message;
    enc28j60.spi.data = &enc28j60_spi;

    /* Enable GPIO A clock. */
    RCC->AHB1ENR |= 0x1;

    /* Configure GPIO mode output for GPIOA.2 (INT) and output for GPIOA.3 (RST). */
    GPIOA->MODER &= ~((GPIO_MODER_MODER0 << (2 * 2)) | (GPIO_MODER_MODER0 << (3 * 2)));
    GPIOA->MODER |= (0x1 << (3 * 2));

    /* Configure output type (PP). */
    GPIOA->OTYPER &= (uint32_t)(~((GPIO_OTYPER_OT_0 << (2 * 2)) | (GPIO_OTYPER_OT_0 << (3 * 2))));

    /* Enable pull-up on GPIOA.2 and GPIOA.3. */
    GPIOA->PUPDR &= (uint32_t)(~(((GPIO_PUPDR_PUPDR0 << (2 * 2)) | (GPIO_PUPDR_PUPDR0 << (3 * 2)))));
    GPIOA->PUPDR |= (0x1 << (2 * 2));

    /* Configure GPIO speed (100MHz). */
    GPIOA->OSPEEDR &= (uint32_t)(~((GPIO_OSPEEDER_OSPEEDR0 << (2 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (3 * 2))));
    GPIOA->OSPEEDR |= ((0x3 << (2 * 2)) | (0x3 << (3 * 2)));

#if (ENC28J60_INT_POLL == FALSE)
    /* Set EXTI line for processing interrupts on GPIOA.2. */
    SYSCFG->EXTICR[(0x2 >> 0x2)] &= (uint32_t)(~(0xF << (0x4 * (0x2 & 0x3))));
    SYSCFG->EXTICR[(0x2 >> 0x2)] |= (0x0 << (0x4 * (0x2 & 0x3)));

    /* Clear EXTI line configuration. */
    EXTI->IMR &= (uint32_t)~(0x4);
    EXTI->EMR &= (uint32_t)~(0x4);
    EXTI->RTSR &= (uint32_t)~(0x4);
    EXTI->FTSR &= (uint32_t)~(0x4);

    /* Enable interrupt mode. */
    *(uint32_t *) (EXTI_BASE + 0x0) |= (0x4);

    /* Enable interrupt for falling edge. */
    *(uint32_t *) (EXTI_BASE + 0xC) |= (0x4);

    /* Set EXT2 IRQ channel priority. */
    NVIC->IP[EXTI2_IRQn] = 2;
#endif

    /* Initialize device APIs. */
#if (ENC28J60_INT_POLL == FALSE)
    enc28j60.enable_interrupts = &enc28j60_stm32f411_enable_interrupt;
    enc28j60.disable_interrupts = &enc28j60_stm32f411_disable_interrupt;
#endif
    enc28j60.interrupt_pin = &enc28j60_stm32f411_interrupt_pin;
    enc28j60.reset = &enc28j60_stm32f411_reset;
    enc28j60.get_mac = &enc28j60_stm32f411_get_mac;

    /* Initialize name for this device. */
    enc28j60.ethernet_device.fs.name = "\\ethernet\\enc28j60";

    /* Initialize SPI parameters. */
    enc28j60.spi.baudrate = ENC28J60_STM32F411_BAUDRATE;
    enc28j60.spi.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);

    /* Do SPI initialization. */
    spi_init(&enc28j60.spi);

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_stm32f411_init */

#if (ENC28J60_INT_POLL == FALSE)
/*
 * enc28j60_stm32f411_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_stm32f411_handle_interrupt(void)
{
    /* Disable interrupt until we process it. */
    enc28j60.flags &= (uint8_t)~(ENC28J60_INT_ENABLE);
    enc28j60_stm32f411_disable_interrupt(&enc28j60);

    /* Handle interrupt for this device. */
    ethernet_interrupt(&enc28j60.ethernet_device);

} /* enc28j60_stm32f411_handle_interrupt */

/*
 * enc28j60_stm32f411_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f411_enable_interrupt(ENC28J60 *device)
{
    /* If we need to enable interrupts for this device. */
    if (device->flags & ENC28J60_INT_ENABLE)
    {
        /* Enable the EXT2 IRQ channel. */
        NVIC->ISER[EXTI2_IRQn >> 0x5] = (uint32_t)0x1 << (EXTI2_IRQn & (uint8_t)0x1F);
    }

} /* enc28j60_stm32f411_enable_interrupt */

/*
 * enc28j60_stm32f411_disable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  disabled.
 * This function will disable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f411_disable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Disable the EXT2 IRQ channel. */
    NVIC->ICER[EXTI2_IRQn >> 0x5] = (uint32_t)0x1 << (EXTI2_IRQn & (uint8_t)0x1F);

} /* enc28j60_stm32f411_disable_interrupt */
#endif

/*
 * enc28j60_stm32f411_interrupt_pin
 * device: ENC28J60 device instance for which we need to query the status of
 * interrupt pin.
 * This function will return the status of interrupt pin.
 */
uint8_t enc28j60_stm32f411_interrupt_pin(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Return if interrupt pin is high. */
    return ((GPIOA->IDR & (1 << 2)) != FALSE);

} /* enc28j60_stm32f411_interrupt_pin */

/*
 * enc28j60_stm32f411_reset
 * device: ENC28J60 device needed to be reset.
 * This function will reset the target enc28j60 device.
 */
void enc28j60_stm32f411_reset(ENC28J60 *device)
{
    FD *fd = (FD)device;

    /* Clear the RST, i.e. GPIOA.3. */
    GPIOA->BSRR |= (1 << (3 + 16));

    /* Release lock for this device. */
    fd_release_lock(fd);

    /* Sleep to wait for target to actually reset. */
    sleep_fms(ENC28J60_STM32F411_RESET_DELAY);

    /* Acquire lock for this device. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Set the RST, i.e. GPIOA.3. */
    GPIOA->BSRR |= (1 << 3);

} /* enc28j60_stm32f411_reset */

/*
 * enc28j60_stm32f411_get_mac
 * @device: Ethernet device instance for which MAC address is needed to
 *  be assigned.
 * @return: Returns the start of random MAC address generated.
 * This function will generate a MAC address using the device serial.
 */
uint8_t *enc28j60_stm32f411_get_mac(ETH_DEVICE *device)
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

} /* enc28j60_stm32f411_get_mac */

#endif /* ETHERNET_ENC28J60 */
