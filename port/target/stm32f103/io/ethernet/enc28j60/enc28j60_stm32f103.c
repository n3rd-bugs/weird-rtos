/*
 * enc28j60_stm32f103.c
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
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <enc28j60.h>
#include <enc28j60_stm32f103.h>
#include <string.h>
#include <spi_stm32f103.h>
#include <net_csum.h>

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;
static STM32F103_SPI enc28j60_spi;

/*
 * enc28j60_stm32f103_init
 * This function will initialize enc28j60 device for stm32f103 platform.
 */
void enc28j60_stm32f103_init(void)
{
    /* Clear the enc28j60 device structures. */
    memset(&enc28j60, 0, sizeof(ENC28J60));
    memset(&enc28j60_spi, 0, sizeof(STM32F103_SPI));

    /* Initialize SPI device data. */
    enc28j60_spi.device_num = 1;
    enc28j60.spi.init = &spi_stm32f103_init;
    enc28j60.spi.slave_select = &spi_stm32f103_slave_select;
    enc28j60.spi.slave_unselect = &spi_stm32f103_slave_unselect;
    enc28j60.spi.msg = &spi_stm32f103_message;
    enc28j60.spi.data = &enc28j60_spi;

    /* Enable clock for GPIOC. */
    RCC->APB2ENR |= RCC_APB2Periph_GPIOC;

    /* Configure GPIO mode input for PC14 (INT) and output for PC15 (RST). */
    GPIOC->CRH &= (uint32_t)(~((0xF << ((14 - 8) << 2)) | (0xF << ((15 - 8) << 2))));
    GPIOC->CRH |= (((GPIO_Mode_IN_FLOATING) & 0xF) << ((14 - 8) << 2)) | (((GPIO_Speed_50MHz | GPIO_Mode_Out_PP) & 0xF) << ((15 - 8) << 2));

#if (ENC28J60_INT_POLL == FALSE)
    /* Enable clock for AFIO. */
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;

    /* Set EXTI line for processing interrupts on PC14 (INT). */
    AFIO->EXTICR[(14 >> 0x2)] &= (uint32_t)(~(0xF << (0x4 * (14 & 0x3))));
    AFIO->EXTICR[(14 >> 0x2)] |= (0x2 << (0x4 * (14 & 0x3)));

    /* Clear EXTI line configuration. */
    EXTI->IMR &= (uint32_t)~(1 << 14);
    EXTI->EMR &= (uint32_t)~(1 << 14);
    EXTI->RTSR &= (uint32_t)~(1 << 14);
    EXTI->FTSR &= (uint32_t)~(1 << 14);

    /* Enable interrupt mode. */
    EXTI->IMR |= (1 << 14);

    /* Enable interrupt for falling edge. */
    EXTI->FTSR |= (1 << 14);

    /* Set EXT15_10 IRQ channel priority. */
    NVIC->IP[EXTI15_10_IRQn] = 0;
#endif

    /* Initialize device APIs. */
#if (ENC28J60_INT_POLL == FALSE)
    enc28j60.enable_interrupts = &enc28j60_stm32f103_enable_interrupt;
    enc28j60.disable_interrupts = &enc28j60_stm32f103_disable_interrupt;
#endif
    enc28j60.interrupt_pin = &enc28j60_stm32f103_interrupt_pin;
    enc28j60.reset = &enc28j60_stm32f103_reset;
    enc28j60.get_mac = &enc28j60_stm32f103_get_mac;

    /* Initialize name for this device. */
    enc28j60.ethernet_device.fs.name = "\\ethernet\\enc28j60";

    /* Initialize SPI parameters. */
    enc28j60.spi.baudrate = ENC28J60_STM32F103_BAUDRATE;
    enc28j60.spi.cfg_flags = (SPI_CFG_MASTER | SPI_CFG_CLK_FIRST_DATA);

    /* Do SPI initialization. */
    spi_init(&enc28j60.spi);

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_stm32f103_init */

#if (ENC28J60_INT_POLL == FALSE)
/*
 * enc28j60_stm32f103_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_stm32f103_handle_interrupt(void)
{
    /* Disable interrupt until we process it. */
    enc28j60.flags &= (uint8_t)~(ENC28J60_INT_ENABLE);
    enc28j60_stm32f103_disable_interrupt(&enc28j60);

    /* Handle interrupt for this device. */
    ethernet_interrupt(&enc28j60.ethernet_device);

} /* enc28j60_stm32f103_handle_interrupt */

/*
 * enc28j60_stm32f103_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f103_enable_interrupt(ENC28J60 *device)
{
    /* If we need to enable interrupts for this device. */
    if (device->flags & ENC28J60_INT_ENABLE)
    {
        /* Enable the EXT15_10 IRQ channel. */
        NVIC->ISER[EXTI15_10_IRQn >> 0x5] = (uint32_t)0x1 << (EXTI15_10_IRQn & (uint8_t)0x1F);
    }

} /* enc28j60_stm32f103_enable_interrupt */

/*
 * enc28j60_stm32f103_disable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  disabled.
 * This function will disable interrupt for given enc28j60 device.
 */
void enc28j60_stm32f103_disable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Disable the EXT15_10 IRQ channel. */
    NVIC->ICER[EXTI15_10_IRQn >> 0x5] = (uint32_t)0x1 << (EXTI15_10_IRQn & (uint8_t)0x1F);

} /* enc28j60_stm32f103_disable_interrupt */
#endif

/*
 * enc28j60_stm32f103_interrupt_pin
 * device: ENC28J60 device instance for which we need to query the status of
 * interrupt pin.
 * This function will return the status of interrupt pin.
 */
uint8_t enc28j60_stm32f103_interrupt_pin(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Return if PC14 (INT) is high. */
    return ((GPIOC->IDR & (1 << 14)) != FALSE);

} /* enc28j60_stm32f103_interrupt_pin */

/*
 * enc28j60_stm32f103_reset
 * device: ENC28J60 device needed to be reset.
 * This function will reset the target enc28j60 device.
 */
void enc28j60_stm32f103_reset(ENC28J60 *device)
{
    FD *fd = (FD)device;

    /* Clear the PC15 (RST). */
    GPIOC->BSRR |= (uint32_t)(1 << (15 + 16));

    /* Release lock for this device. */
    fd_release_lock(fd);

    /* Sleep to wait for target to actually reset. */
    sleep_fms(ENC28J60_STM32F103_RESET_DELAY);

    /* Acquire lock for this device. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Set the PC15 (RST). */
    GPIOC->BSRR |= (1 << 15);

} /* enc28j60_stm32f103_reset */

/*
 * enc28j60_stm32f103_get_mac
 * @device: Ethernet device instance for which MAC address is needed to
 *  be assigned.
 * @return: Returns the start of random MAC address generated.
 * This function will generate a MAC address using the device serial.
 */
uint8_t *enc28j60_stm32f103_get_mac(ETH_DEVICE *device)
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

} /* enc28j60_stm32f103_get_mac */

#endif /* ETHERNET_ENC28J60 */
