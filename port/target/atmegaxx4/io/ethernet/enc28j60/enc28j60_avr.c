/*
 * enc28j60_avr.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
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
#include <avr/boot.h>
#include <enc28j60.h>
#include <enc28j60_avr.h>
#include <net_csum.h>
#include <string.h>
#if (ENC28J60_USE_SPI_BB == TRUE)
#include <spi_bb_avr.h>
#else
#include <spi_avr.h>
#endif

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;
#if (ENC28J60_USE_SPI_BB == TRUE)
static SPI_BB_AVR spi_bb_enc28j60;
#endif

/*
 * enc28j60_avr_init
 * This function will initialize enc28j60 device for avr platform.
 */
void enc28j60_avr_init(void)
{
    /* Clear the enc28j60 device structure. */
    memset(&enc28j60, 0, sizeof(ENC28J60));

    /* Set RST (PD.4) as output and INT (PD.2) as input. */
    DDRD |= (1 << 4);
    DDRD &= (uint8_t)~(1 << 2);

#if (ENC28J60_INT_POLL == FALSE)
    /* Disable INT0 interrupt. */
    EIMSK &= (uint8_t)~(1 << 0);

    /* Setup INT0 to raise an interrupt on falling edge. */
    EICRA &= (uint8_t)~(0x03 << 0);
    EICRA |= (0x02 << 0);
#endif

    /* Initialize device APIs. */
#if (ENC28J60_INT_POLL == FALSE)
    enc28j60.enable_interrupts = &enc28j60_avr_enable_interrupt;
    enc28j60.disable_interrupts = &enc28j60_avr_disable_interrupt;
#endif
    enc28j60.interrupt_pin = &enc28j60_avr_interrupt_pin;
    enc28j60.reset = &enc28j60_avr_reset;
    enc28j60.get_mac = &enc28j60_avr_get_mac;

    /* Hook-up SPI for this device. */
#if (ENC28J60_USE_SPI_BB == TRUE)
    /* Populate the SPI bit-bang interface. */
    spi_bb_enc28j60.pin_num_ss = ENC28J60_AVR_SPI_SS_BB;
    spi_bb_enc28j60.pin_num_mosi = ENC28J60_AVR_SPI_MOSI_BB;
    spi_bb_enc28j60.pin_num_miso = ENC28J60_AVR_SPI_MISO_BB;
    spi_bb_enc28j60.pin_num_sclk = ENC28J60_AVR_SPI_SCLK_BB;
    spi_bb_enc28j60.pin_ss = ENC28J60_AVR_SPI_PIN_SS_BB;
    spi_bb_enc28j60.pin_mosi = ENC28J60_AVR_SPI_PIN_MOSI_BB;
    spi_bb_enc28j60.pin_miso = ENC28J60_AVR_SPI_PIN_MISO_BB;
    spi_bb_enc28j60.pin_sclk = ENC28J60_AVR_SPI_PIN_SCLK_BB;
    spi_bb_enc28j60.ddr_ss = ENC28J60_AVR_SPI_DDR_SS_BB;
    spi_bb_enc28j60.ddr_mosi = ENC28J60_AVR_SPI_DDR_MOSI_BB;
    spi_bb_enc28j60.ddr_miso = ENC28J60_AVR_SPI_DDR_MISO_BB;
    spi_bb_enc28j60.ddr_sclk = ENC28J60_AVR_SPI_DDR_SCLK_BB;
    spi_bb_enc28j60.port_ss = ENC28J60_AVR_SPI_PORT_SS_BB;
    spi_bb_enc28j60.port_mosi = ENC28J60_AVR_SPI_PORT_MOSI_BB;
    spi_bb_enc28j60.port_miso = ENC28J60_AVR_SPI_PORT_MISO_BB;
    spi_bb_enc28j60.port_sclk = ENC28J60_AVR_SPI_PORT_SCLK_BB;

    /* Initialize enc28j60 SPI device. */
    enc28j60.spi.data = &spi_bb_enc28j60;
    enc28j60.spi.init = &spi_bb_avr_init;
    enc28j60.spi.slave_select = &spi_bb_avr_slave_select;
    enc28j60.spi.slave_unselect = &spi_bb_avr_slave_unselect;
    enc28j60.spi.msg = &spi_bb_avr_message;
#else
    /* Initialize enc28j60 SPI device. */
    enc28j60.spi.init = &spi_avr_init;
    enc28j60.spi.slave_select = &spi_avr_slave_select;
    enc28j60.spi.slave_unselect = &spi_avr_slave_unselect;
    enc28j60.spi.msg = &spi_avr_message;
#endif

    /* Initialize name for this device. */
    enc28j60.ethernet_device.fs.name = "\\ethernet\\enc28j60";

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_avr_init */

#if (ENC28J60_INT_POLL == FALSE)
/*
 * enc28j60_avr_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_avr_handle_interrupt(void)
{
    /* Disable interrupt until we process it. */
    enc28j60.flags &= (uint8_t)~(ENC28J60_INT_ENABLE);
    enc28j60.disable_interrupts(&enc28j60);

    /* Handle interrupt for this device. */
    ethernet_interrupt(&enc28j60.ethernet_device);

} /* enc28j60_avr_handle_interrupt */

/*
 * enc28j60_avr_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_avr_enable_interrupt(ENC28J60 *device)
{
    /* If we really want to enable interrupt for the device. */
    if (device->flags & ENC28J60_INT_ENABLE)
    {
        /* Enable INT0 to process enc28j60 device interrupts. */
        EIMSK |= (1 << 0);
    }

} /* enc28j60_avr_enable_interrupt */

/*
 * enc28j60_avr_disable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  disabled.
 * This function will disable interrupt for given enc28j60 device.
 */
void enc28j60_avr_disable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Disable INT0 interrupt. */
    EIMSK &= (uint8_t)~(1 << 0);

} /* enc28j60_avr_disable_interrupt */
#endif

/*
 * enc28j60_avr_interrupt_pin
 * device: ENC28J60 device instance for which we need to query the status of
 * interrupt pin.
 * This function will return the status of interrupt pin.
 */
uint8_t enc28j60_avr_interrupt_pin(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Return if interrupt pin is high. */
    return ((PIND & (1 << 2)) != FALSE);

} /* enc28j60_avr_interrupt_pin */

/*
 * enc28j60_avr_reset
 * device: ENC28J60 device needed to be reset.
 * This function will reset the target enc28j60 device.
 */
void enc28j60_avr_reset(ENC28J60 *device)
{
    FD *fd = (FD)device;

    /* Clear the RST, i.e. PD.4. */
    PORTD &= (uint8_t)~(1 << 4);

    /* Release lock for this device. */
    fd_release_lock(fd);

    /* Sleep to wait for target to actually reset. */
    sleep_fms(ENC28J60_AVR_RESET_DELAY);

    /* Acquire lock for this device. */
    ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Set the RST, i.e. PD.4. */
    PORTD |= (1 << 4);

} /* enc28j60_avr_reset */

/*
 * enc28j60_avr_get_mac
 * @device: Ethernet device instance for which MAC address is needed to
 *  be assigned.
 * @return: Returns the start of random MAC address generated.
 * This function will generate a MAC address using the device serial.
 */
uint8_t *enc28j60_avr_get_mac(ETH_DEVICE *device)
{
    /* Push ENC28j60 OUI bytes. */
    device->mac[0] = ENC28J60_OUI_B0;
    device->mac[1] = ENC28J60_OUI_B1;
    device->mac[2] = ENC28J60_OUI_B2;

    /* Assign remaining MAC address using the device serial. */
    device->mac[3] = NET_CSUM_BYTE(NET_CSUM_BYTE(boot_signature_byte_get(0x000E), boot_signature_byte_get(0x000F)), boot_signature_byte_get(0x0010));
    device->mac[4] = NET_CSUM_BYTE(NET_CSUM_BYTE(boot_signature_byte_get(0x0011), boot_signature_byte_get(0x0012)), boot_signature_byte_get(0x0013));
    device->mac[5] = NET_CSUM_BYTE(NET_CSUM_BYTE(boot_signature_byte_get(0x0015), boot_signature_byte_get(0x0016)), boot_signature_byte_get(0x0017));

    /* Return the generated MAC address. */
    return (device->mac);

} /* enc28j60_avr_get_mac */

#endif /* ETHERNET_ENC28J60 */
