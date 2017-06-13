/*
 * enc28j60_atmega644p.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <os.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60
#include <avr/boot.h>
#include <enc28j60.h>
#include <enc28j60_atmega644p.h>
#include <net_csum.h>
#include <string.h>
#if (ENC28J60_USE_SPI_BB == TRUE)
#include <spi_bb_atmega644p.h>
#else
#include <spi_atmega644p.h>
#endif

/* ENC28J60 device instance. */
static ENC28J60 enc28j60;
#if (ENC28J60_USE_SPI_BB == TRUE)
static SPI_BB_AVR spi_bb_enc28j60;
#endif

/*
 * enc28j60_atmega644p_init
 * This function will initialize enc28j60 device for atmega644p platform.
 */
void enc28j60_atmega644p_init()
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
    enc28j60.enable_interrupts = &enc28j60_atmega644p_enable_interrupt;
    enc28j60.disable_interrupts = &enc28j60_atmega644p_disable_interrupt;
#endif
    enc28j60.interrupt_pin = &enc28j60_atmega644p_interrupt_pin;
    enc28j60.reset = &enc28j60_atmega644p_reset;
    enc28j60.get_mac = &enc28j60_atmega644p_get_mac;

    /* Hook-up SPI for this device. */
#if (ENC28J60_USE_SPI_BB == TRUE)
    /* Populate the SPI bit-bang interface. */
    spi_bb_enc28j60.pin_num_ss = 4;
    spi_bb_enc28j60.pin_num_mosi = 5;
    spi_bb_enc28j60.pin_num_miso = 6;
    spi_bb_enc28j60.pin_num_sclk = 7;
    spi_bb_enc28j60.pin_miso = spi_bb_enc28j60.pin_mosi = spi_bb_enc28j60.pin_ss = spi_bb_enc28j60.pin_sclk = 0x03;
    spi_bb_enc28j60.ddr_miso = spi_bb_enc28j60.ddr_mosi = spi_bb_enc28j60.ddr_ss = spi_bb_enc28j60.ddr_sclk = 0x04;
    spi_bb_enc28j60.port_miso = spi_bb_enc28j60.port_mosi = spi_bb_enc28j60.port_ss = spi_bb_enc28j60.port_sclk = 0x05;

    /* Initialize enc28j60 SPI device. */
    enc28j60.spi.data = &spi_bb_enc28j60;
    enc28j60.spi.init = &spi_bb_atmega644_init;
    enc28j60.spi.slave_select = &spi_bb_atmega644_slave_select;
    enc28j60.spi.slave_unselect = &spi_bb_atmega644_slave_unselect;
    enc28j60.spi.msg = &spi_bb_atmega644_message;
#else
    /* Initialize enc28j60 SPI device. */
    enc28j60.spi.init = &spi_atmega644_init;
    enc28j60.spi.slave_select = &spi_atmega644_slave_select;
    enc28j60.spi.slave_unselect = &spi_atmega644_slave_unselect;
    enc28j60.spi.msg = &spi_atmega644_message;
#endif

    /* Initialize name for this device. */
    enc28j60.ethernet_device.fs.name = "\\ethernet\\enc28j60";

    /* Do enc28j60 initialization. */
    enc28j60_init(&enc28j60);

} /* enc28j60_atmega644p_init */

#if (ENC28J60_INT_POLL == FALSE)
/*
 * enc28j60_atmega644p_handle_interrupt
 * This function will handle interrupt for given enc28j60 device.
 */
void enc28j60_atmega644p_handle_interrupt()
{
    /* Disable interrupt until we process it. */
    enc28j60.flags &= (uint8_t)~(ENC28J60_INT_ENABLE);
    enc28j60.disable_interrupts(&enc28j60);

    /* Handle interrupt for this device. */
    ethernet_interrupt(&enc28j60.ethernet_device);

} /* enc28j60_atmega644p_handle_interrupt */

/*
 * enc28j60_atmega644p_enable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  enabled.
 * This function will enable interrupt for given enc28j60 device.
 */
void enc28j60_atmega644p_enable_interrupt(ENC28J60 *device)
{
    /* If we really want to enable interrupt for the device. */
    if (device->flags & ENC28J60_INT_ENABLE)
    {
        /* Enable INT0 to process enc28j60 device interrupts. */
        EIMSK |= (1 << 0);
    }

} /* enc28j60_atmega644p_enable_interrupt */

/*
 * enc28j60_atmega644p_disable_interrupt
 * device: ENC28J60 device instance for which interrupts are needed to be
 *  disabled.
 * This function will disable interrupt for given enc28j60 device.
 */
void enc28j60_atmega644p_disable_interrupt(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Disable INT0 interrupt. */
    EIMSK &= (uint8_t)~(1 << 0);

} /* enc28j60_atmega644p_disable_interrupt */
#endif

/*
 * enc28j60_atmega644p_interrupt_pin
 * device: ENC28J60 device instance for which we need to query the status of
 * interrupt pin.
 * This function will return the status of interrupt pin.
 */
uint8_t enc28j60_atmega644p_interrupt_pin(ENC28J60 *device)
{
    /* For now unused. */
    UNUSED_PARAM(device);

    /* Return if interrupt pin is high. */
    return ((PIND & (1 << 2)) != FALSE);

} /* enc28j60_atmega644p_interrupt_pin */

/*
 * enc28j60_atmega644p_reset
 * device: ENC28J60 device needed to be reset.
 * This function will reset the target enc28j60 device.
 */
void enc28j60_atmega644p_reset(ENC28J60 *device)
{
    FD *fd = (FD)device;

    /* Clear the RST, i.e. PD.4. */
    PORTD &= (uint8_t)~(1 << 4);

    /* Release lock for this device. */
    fd_release_lock(fd);

    /* Sleep to wait for target to actually reset. */
    sleep_ms(ENC28J60_ATMEGA644P_RESET_DELAY);

    /* Acquire lock for this device. */
    OS_ASSERT(fd_get_lock(fd) != SUCCESS);

    /* Set the RST, i.e. PD.4. */
    PORTD |= (1 << 4);

} /* enc28j60_atmega644p_reset */

/*
 * enc28j60_atmega644p_get_mac
 * @device: Ethernet device instance for which MAC address is needed to
 *  be assigned.
 * @return: Returns the start of random MAC address generated.
 * This function will generate a MAC address using the device serial.
 */
uint8_t *enc28j60_atmega644p_get_mac(ETH_DEVICE *device)
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

} /* enc28j60_atmega644p_get_mac */

#endif /* ETHERNET_ENC28J60 */
