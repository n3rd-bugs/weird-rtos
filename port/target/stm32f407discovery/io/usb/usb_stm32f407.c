/*
 * usb_stm32f407.c
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

#ifdef CONFIG_USB
#include <usb_stm32f407.h>

/*
 * usb_stm32f407_init
 * This function will initialize USB devices for STM32F407 platform.
 */
void usb_stm32f407_init()
{
#ifdef USB_FUNCTION
    /* Initialize USB function devices. */
    usb_function_stm32f407_init();
#endif
} /* usb_stm32f407_init */

/*
 * usb_stm32f407_hw_initilaize
 * @usb_device: USB device instance.
 * This function will initializes USB hardware for STM32F407.
 */
void usb_stm32f407_hw_initilaize(USB_STM32F407_HANDLE *usb_device)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(usb_device);

#ifdef STM32F407_USB_FS

    /* Enable GPIO A clock. */
    RCC->AHB1ENR |= 0x00000001;

    /* Configure GPIO mode (alternate function). */
    GPIOA->MODER &= ~((GPIO_MODER_MODER0 << (8 * 2)) | (GPIO_MODER_MODER0 << (9 * 2)) | (GPIO_MODER_MODER0 << (10 * 2)) | (GPIO_MODER_MODER0 << (11 * 2)) | (GPIO_MODER_MODER0 << (12 * 2)));
    GPIOA->MODER |= ((0x02 << (8 * 2)) | (0x02 << (9 * 2)) | (0x02 << (10 * 2)) | (0x02 << (11 * 2)) | (0x02 << (12 * 2)));

    /* Configure GPIO speed (100MHz). */
    GPIOA->OSPEEDR &= ~((GPIO_OSPEEDER_OSPEEDR0 << (8 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (9 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (11 * 2)) | (GPIO_OSPEEDER_OSPEEDR0 << (12 * 2)));
    GPIOA->OSPEEDR |= ((0x03 << (8 * 2)) | (0x03 << (9 * 2)) | (0x03 << (10 * 2)) | (0x03 << (11 * 2)) | (0x03 << (12 * 2)));

    /* Configure output type (PP, OD for GPIOA.10). */
    GPIOA->OTYPER &= ~((GPIO_OTYPER_OT_0 << (8 * 2)) | (GPIO_OTYPER_OT_0 << (9 * 2)) | (GPIO_OTYPER_OT_0 << (10 * 2)) | (GPIO_OTYPER_OT_0 << (11 * 2)) | (GPIO_OTYPER_OT_0 << (12 * 2)));
    GPIOA->OTYPER |= (0x01 << (10 * 2));

    /* Disable GPIO pull ups except for GPIOA.10. */
    GPIOA->PUPDR &= ~((GPIO_PUPDR_PUPDR0 << (8 * 2)) | (GPIO_PUPDR_PUPDR0 << (9 * 2)) | (GPIO_PUPDR_PUPDR0 << (10 * 2)) | (GPIO_PUPDR_PUPDR0 << (11 * 2)) | (GPIO_PUPDR_PUPDR0 << (12 * 2)));
    GPIOA->PUPDR |= (0x01 << (10 * 2));

    /* Enable FS mode on GPIOA.8. */
    GPIOA->AFR[((uint8_t)0x08) >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)((uint8_t)0x08) & (uint32_t)0x07) * 4));
    GPIOA->AFR[((uint8_t)0x08) >> 0x03] |= ((uint32_t)(((uint8_t)0xA)) << ((uint32_t)((uint32_t)((uint8_t)0x08) & (uint32_t)0x07) * 4));

    /* Enable FS mode on GPIOA.9. */
    GPIOA->AFR[((uint8_t)0x09) >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)((uint8_t)0x09) & (uint32_t)0x07) * 4));
    GPIOA->AFR[((uint8_t)0x09) >> 0x03] |= ((uint32_t)(((uint8_t)0xA)) << ((uint32_t)((uint32_t)((uint8_t)0x09) & (uint32_t)0x07) * 4));

    /* Enable FS mode on GPIOA.10. */
    GPIOA->AFR[((uint8_t)0x0A) >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)((uint8_t)0x0A) & (uint32_t)0x07) * 4));
    GPIOA->AFR[((uint8_t)0x0A) >> 0x03] |= ((uint32_t)(((uint8_t)0xA)) << ((uint32_t)((uint32_t)((uint8_t)0x0A) & (uint32_t)0x07) * 4));

    /* Enable FS mode on GPIOA.11. */
    GPIOA->AFR[((uint8_t)0x0B) >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)((uint8_t)0x0B) & (uint32_t)0x07) * 4));
    GPIOA->AFR[((uint8_t)0x0B) >> 0x03] |= ((uint32_t)(((uint8_t)0xA)) << ((uint32_t)((uint32_t)((uint8_t)0x0B) & (uint32_t)0x07) * 4));

    /* Enable FS mode on GPIOA.12. */
    GPIOA->AFR[((uint8_t)0x0C) >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)((uint8_t)0x0C) & (uint32_t)0x07) * 4));
    GPIOA->AFR[((uint8_t)0x0C) >> 0x03] |= ((uint32_t)(((uint8_t)0xA)) << ((uint32_t)((uint32_t)((uint8_t)0x0C) & (uint32_t)0x07) * 4));

    /* Enable system configuration clock. */
    RCC->APB2ENR |= 0x00004000;

    /* Enable OTG FS core clock. */
    RCC->AHB2ENR |= 0x00000080;
#else
#error "Other modes are not supported yet."
#endif

    /* Enable the AHB PWR peripheral clock. */
    RCC->AHB1ENR |= 0x10000000;

    /* Clear pending interrupts on line 0. */
    EXTI->PR = 0x00001;

} /* usb_stm32f407_hw_initilaize */

/*
 * usb_stm32f407_enable_interrupt
 * @usb_device: USB device instance.
 * This function enables USB interrupts.
 */
void usb_stm32f407_enable_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(usb_device);

    /* Enable OTG FS interrupt. */
    NVIC->ISER[OTG_FS_IRQn >> 0x05] =
    (uint32_t)0x01 << (OTG_FS_IRQn & (uint8_t)0x1F);

} /* usb_stm32f407_enable_interrupt */

/*
 * usb_stm32f407_select_core
 * @usb_device: USB device instance.
 * This function will select a core for this device.
 */
uint32_t usb_stm32f407_select_core(USB_STM32F407_HANDLE *usb_device)
{
    uint32_t i, baseAddress = 0;

    /* Disable DMA by default. */
    usb_device->cfg.dma_enable = 0;

    /* Select FS mode by default. */
    usb_device->cfg.speed           = USB_STM32F407_SPEED_FULL;
    usb_device->cfg.max_packet_size = USB_STM32F407_FS_MAX_PACKET_SIZE;

#if defined(STM32F407_USB_FS_CORE)

    /* Initialize FS core configuration. */
    baseAddress                     = USB_STM32F407_FS_BASE_ADDR;
    usb_device->cfg.host_channels   = 8;
    usb_device->cfg.dev_endpoints   = 4;

#ifdef STM32F407_USB_FS_SOF_OUTPUT_ENABLED
    usb_device->cfg.sof_output       = 1;
#endif

#ifdef STM32F407_USB_FS_LOW_PWR_MGMT_SUPPORT
    usb_device->cfg.low_power        = 1;
#endif

#elif defined(STM32F407_USB_HS_CORE)

    /* Initialize HS core configuration. */
    baseAddress                     = USB_STM32F407_HS_BASE_ADDR;
    usb_device->cfg.coreID          = USB_HS_CORE;
    usb_device->cfg.host_channels   = 12;
    usb_device->cfg.dev_endpoints   = 6;

#if defined(STM32F407_USB_ULPI_PHY_ENABLED)
    usb_device->cfg.phy_itface       = STM32F407_USB_ULPI_PHY;
#elif defined(STM32F407_USB_EMBEDDED_PHY_ENABLED)
    usb_device->cfg.phy_itface       = STM32F407_USB_EMBEDDED_PHY;
#elif defined(STM32F407_USB_I2C_PHY_ENABLED)
    usb_device->cfg.phy_itface       = STM32F407_USB_I2C_PHY;
#endif

#ifdef STM32F407_USB_HS_DMA_ENABLED
    usb_device->cfg.dma_enable       = 1;
#endif

#ifdef STM32F407_USB_HS_SOF_OUTPUT_ENABLED
    usb_device->cfg.sof_output       = 1;
#endif

#ifdef STM32F407_USB_HS_LOW_PWR_MGMT_SUPPORT
    usb_device->cfg.low_power        = 1;
#endif
#endif /* STM32F407_USB_FS_CORE */

    /* Initialize USB registers for the selected core. */
    usb_device->regs.GREGS = (USB_STM32F407_GREGS *)(baseAddress + USB_STM32F407_CORE_GLOBAL_OFFSET);
    usb_device->regs.DREGS = (USB_STM32F407_DREGS *)(baseAddress + USB_STM32F407_DEV_GLOBAL_OFFSET);

    /* Initialize endpoint registers. */
    for (i = 0; i < usb_device->cfg.dev_endpoints; i++)
    {
        usb_device->regs.INEP_REGS[i]  = (USB_STM32F407_INEPREGS *)(baseAddress + USB_STM32F407_DEV_IN_EP_OFFSET + (i * USB_STM32F407_EP_OFFSET));
        usb_device->regs.OUTEP_REGS[i] = (USB_STM32F407_OUTEPREGS *)(baseAddress + USB_STM32F407_DEV_OUT_EP_OFFSET + (i * USB_STM32F407_EP_OFFSET));
    }

    usb_device->regs.HREGS = (USB_STM32F407_HREGS *)(baseAddress + USB_STM32F407_HOST_GLOBAL_OFFSET);
    usb_device->regs.HPRT0 = (uint32_t *)(baseAddress + USB_STM32F407_HOST_PORT_OFFSET);

    /* Initialize channel data and FIFO registers. */
    for (i = 0; i < usb_device->cfg.host_channels; i++)
    {
        usb_device->regs.HC_REGS[i] = (USB_STM32F407_HC_REGS *)(baseAddress + USB_STM32F407_HOST_CHAN_OFFSET + (i * USB_STM32F407_CHAN_OFFSET));
        usb_device->regs.DFIFO[i] = (uint32_t *)(baseAddress + USB_STM32F407_DATA_FIFO_OFFSET + (i * USB_STM32F407_DATA_FIFO_SIZE));
    }

    usb_device->regs.PCGCCTL = (uint32_t *)(baseAddress + USB_STM32F407_PCGCCTL_OFFSET);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_select_core */

/*
 * usb_stm32f407_core_initialize
 * @usb_device: USB device instance.
 * This function will initialize selected core for this device.
 */
uint32_t usb_stm32f407_core_initialize(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_GUSBCFG usbcfg;
    USB_STM32F407_GCCFG gccfg;
#ifdef STM32F407_USB_I2C_PHY_ENABLED
    USB_STM32F407_GI2CCTL i2cctl;
#endif
    USB_STM32F407_GAHBCFG ahbcfg;

    usbcfg.d32 = 0;
    gccfg.d32 = 0;
    ahbcfg.d32 = 0;

#ifdef STM32F407_USB_ULPI_PHY_ENABLED
    /* We are using ULPI interface. */
    gccfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GCCFG);
    gccfg.b.pwdn = 0;

    if (usb_device->cfg.sof_output)
    {
        gccfg.b.sofouten = 1;
    }

    OS_WRITE_REG32 (&usb_device->regs.GREGS->GCCFG, gccfg.d32);

    /* Initialize ULP interface. */
    usbcfg.d32 = 0;
    usbcfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GUSBCFG);

    usbcfg.b.physel             = 0;    /* HS Interface */
#ifdef STM32F407_USB_INTERNAL_VBUS_ENABLED
    usbcfg.b.ulpi_ext_vbus_drv  = 0;    /* Use internal VBUS */
#else
    usbcfg.b.ulpi_ext_vbus_drv  = 1;    /* Use external VBUS */
#endif
    usbcfg.b.term_sel_dl_pulse  = 0;    /* Data line pulsing using utmi_txvalid */
    usbcfg.b.ulpi_utmi_sel      = 1;    /* ULPI seleInterfacect */

    usbcfg.b.phyif              = 0;    /* 8 bits */
    usbcfg.b.ddrsel             = 0;    /* single data rate */

    usbcfg.b.ulpi_fsls          = 0;
    usbcfg.b.ulpi_clk_sus_m     = 0;
    OS_WRITE_REG32(&usb_device->regs.GREGS->GUSBCFG, usbcfg.d32);

    /* Reset after a PHY select. */
    usb_stm32f407_core_reset(usb_device);

    /* If we are using DMA interface. */
    if (usb_device->cfg.dma_enable == 1)
    {
        ahbcfg.b.hburstlen = 5;     /* 64 x 32-bits*/
        ahbcfg.b.dmaenable = 1;
        OS_WRITE_REG32(&usb_device->regs.GREGS->GAHBCFG, ahbcfg.d32);
    }
#else

    usbcfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GUSBCFG);
    usbcfg.b.physel = 1;    /* FS Interface */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GUSBCFG, usbcfg.d32);

    /* Reset after a PHY select. */
    usb_stm32f407_core_reset(usb_device);

    /* Deactivate the power down. */
    gccfg.d32 = 0;
    gccfg.b.pwdn = 1;

#ifdef STM32F407_USB_I2C_PHY_ENABLED
    /* Enable the I2C interface. */
    gccfg.b.i2cifen = 1;
#endif

    gccfg.b.vbussensingA = 1 ;
    gccfg.b.vbussensingB = 1 ;
#ifndef STM32F407_USB_VBUS_SENSING_ENABLED
    gccfg.b.disablevbussensing = 1;
#endif

    if (usb_device->cfg.sof_output)
    {
        gccfg.b.sofouten = 1;
    }

    OS_WRITE_REG32(&usb_device->regs.GREGS->GCCFG, gccfg.d32);

    /* Get current configuration. */
    usbcfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GUSBCFG);

#ifdef STM32F407_USB_I2C_PHY_ENABLED
    /* Program I2C interface. */
    usbcfg.b.otgutmifssel = 1;
#endif

    OS_WRITE_REG32(&usb_device->regs.GREGS->GUSBCFG, usbcfg.d32);

#ifdef STM32F407_USB_I2C_PHY_ENABLED
    /* Enable I2C interface. */
    i2cctl.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GI2CCTL);
    i2cctl.b.i2cdevaddr = 1;
    i2cctl.b.i2cen = 0;
    i2cctl.b.dat_se0 = 1;
    i2cctl.b.addr = 0x2D;
    OS_WRITE_REG32(&usb_device->regs.GREGS->GI2CCTL, i2cctl.d32);

    USB_OTG_BSP_mDelay(200);

    i2cctl.b.i2cen = 1;
    OS_WRITE_REG32(&usb_device->regs.GREGS->GI2CCTL, i2cctl.d32);
    USB_OTG_BSP_mDelay(200);
#endif
#endif

    /* If we are using DMA interface. */
    if (usb_device->cfg.dma_enable == 1)
    {
        ahbcfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GAHBCFG);
        ahbcfg.b.hburstlen = 5;     /* 64 x 32-bits*/
        ahbcfg.b.dmaenable = 1;
        OS_WRITE_REG32(&usb_device->regs.GREGS->GAHBCFG, ahbcfg.d32);
    }

#ifdef STM32F407_USB_OTG_MODE
    /* Initialize OTG features. */
    usbcfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GUSBCFG);
    usbcfg.b.hnpcap = 1;
    usbcfg.b.srpcap = 1;
    OS_WRITE_REG32(&usb_device->regs.GREGS->GUSBCFG, usbcfg.d32);

    /* Enable interrupts. */
    usb_stm32f407_enable_common_interrupt(usb_device);
#endif

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_core_initialize */

/*
 * usb_stm32f407_enable_common_interrupt
 * @usb_device: USB device instance.
 * This function will enable common USB interrupts.
 */
void usb_stm32f407_enable_common_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_GINTMSK int_mask;

    /* Initialize configuration. */
    int_mask.d32 = 0;

#ifndef STM32F407_USB_OTG_MODE
    /* Clear any pending USB OTG interrupts. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GOTGINT, 0xFFFFFFFF);
#endif

    /* Clear any pending global interrupts. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GINTSTS, 0xFFFFFFFF);

    /* Enable the USB interrupts. */
    int_mask.b.wkupintr = 1;
    int_mask.b.usbsuspend = 1;

#ifdef STM32F407_USB_OTG_MODE
    /* Enable the USB OTG interrupts. */
    int_mask.b.otgintr = 1;
    int_mask.b.sessreqintr = 1;
    int_mask.b.conidstschng = 1;
#endif

    /* Update interrupt configuration. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GINTMSK, int_mask.d32);

} /* usb_stm32f407_enable_common_interrupt */

/*
 * usb_stm32f407_enable_common_interrupt
 * @usb_device: USB device instance.
 * This function will do a soft USB reset.
 */
uint32_t usb_stm32f407_core_reset(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_GRSTCTL greset;

    greset.d32 = 0;

    /* Wait for AHB master IDLE state. */
    do
    {
        greset.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GRSTCTL);

    } while (greset.b.ahbidle == 0);

    /* Core soft reset. */
    greset.b.csftrst = 1;
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRSTCTL, greset.d32);

    /* Wait for USB core to resume. */
    do
    {
        greset.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GRSTCTL);

    } while (greset.b.csftrst == 1);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_core_reset */

/*
 * usb_stm32f407_write_packet
 * @usb_device: USB device instance.
 * @buffer: Buffer from which data is needed to be sent.
 * @endpoint: Endpoint on which data is needed to be sent.
 * @length: Number of bytes needed to be sent.
 * This function will write a packet on a given USB endpoint.
 */
uint32_t usb_stm32f407_write_packet(USB_STM32F407_HANDLE *usb_device, uint8_t *buffer, uint8_t endpoint, uint32_t length)
{
    /* Save the end pointer. */
    uint8_t *end_ptr = (buffer + length);

    /* If we are not using DMA. */
    if (usb_device->cfg.dma_enable == 0)
    {
        /* While we need to send some data. */
        for ( ;(end_ptr > buffer); buffer += 4)
        {
            /* Write a word on the FIFO. */
            OS_WRITE_REG32(usb_device->regs.DFIFO[endpoint], *((uint32_t *)buffer));
        }
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_write_packet */

/*
 * usb_stm32f407_read_packet
 * @usb_device: USB device instance.
 * @buffer: Buffer in which data is needed to be copied.
 * @length: Number of bytes to copy.
 * This function will read a packet from USB.
 */
uint32_t usb_stm32f407_read_packet(USB_STM32F407_HANDLE *usb_device, uint8_t *buffer, uint32_t length)
{
    /* Save the end pointer. */
    uint8_t *end_ptr = (buffer + length);

    /* While we need to read some data. */
    for ( ;(end_ptr > buffer); buffer += 4)
    {
        /* Read a word from the FIFO. */
        *(uint32_t *)buffer = OS_READ_REG32(usb_device->regs.DFIFO[0]);
    }

    /* Return number of bytes read. */
    return (length);

} /* usb_stm32f407_read_packet */

/*
 * usb_stm32f407_enable_global_interrupt
 * @usb_device: USB device instance.
 * Enables controller global interrupt.
 */
uint32_t usb_stm32f407_enable_global_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_GAHBCFG ahbcfg;

    /* Initialize AHB interrupt configuration. */
    ahbcfg.d32 = 0;

    /* Enable global interrupts. */
    ahbcfg.b.glblintrmsk = 1;

    /* Update the configuration. */
    OS_MASK_REG32(&usb_device->regs.GREGS->GAHBCFG, 0, ahbcfg.d32);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_enable_global_interrupt */

/*
 * usb_stm32f407_disable_global_interrupt
 * @usb_device: USB device instance.
 * Disable controller global interrupt.
 */
uint32_t usb_stm32f407_disable_global_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_GAHBCFG ahbcfg;

    /* Initialize AHB interrupt configuration. */
    ahbcfg.d32 = 0;

    /* Disable global interrupts. */
    ahbcfg.b.glblintrmsk = 1;

    /* Update the configuration. */
    OS_MASK_REG32(&usb_device->regs.GREGS->GAHBCFG, ahbcfg.d32, 0);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_disable_global_interrupt */

/*
 * usb_stm32f407_flush_tx_fifo
 * @usb_device: USB device instance.
 * @num: TX FIFO number.
 * Flush a TX FIFO.
 */
uint32_t usb_stm32f407_flush_tx_fifo(USB_STM32F407_HANDLE *usb_device, uint32_t num)
{
    volatile USB_STM32F407_GRSTCTL greset;

    /* Initialize the configuration. */
    greset.d32 = 0;
    greset.b.txfflsh = 1;
    greset.b.txfnum = num & (MASK_N_BITS(5));

    /* Write the configuration. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRSTCTL, greset.d32);

    do
    {
        /* Check if FIFO is flushed. */
        greset.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GRSTCTL);

    } while (greset.b.txfflsh == 1);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_flush_tx_fifo */

/*
 * usb_stm32f407_flush_rx_fifo
 * @usb_device: USB device instance.
 * Flushes RX FIFO.
 */
uint32_t usb_stm32f407_flush_rx_fifo(USB_STM32F407_HANDLE *usb_device)
{
    volatile USB_STM32F407_GRSTCTL greset;

    /* Initialize the configuration. */
    greset.d32 = 0;
    greset.b.rxfflsh = 1;

    /* Write the configuration. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRSTCTL, greset.d32);

    do
    {
        /* Check if FIFO is flushed. */
        greset.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GRSTCTL);

    } while (greset.b.rxfflsh == 1);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_flush_rx_fifo */

/*
 * usb_stm32f407_set_current_mode
 * @usb_device: USB device instance.
 * @mode: Mode to set.
 * Set USB mode.
 */
uint32_t usb_stm32f407_set_current_mode(USB_STM32F407_HANDLE *usb_device, uint8_t mode)
{
    USB_STM32F407_GUSBCFG usbcfg;

    /* Read current configuration. */
    usbcfg.d32 = OS_READ_REG32(&usb_device->regs.GREGS->GUSBCFG);

    /* Initialize the configuration. */
    usbcfg.b.force_host = 0;
    usbcfg.b.force_dev = 0;

    if (mode == USB_STM32F407_HOST_MODE)
    {
        usbcfg.b.force_host = 1;
    }
    else if (mode == USB_STM32F407_DEVICE_MODE)
    {
        usbcfg.b.force_dev = 1;
    }

    /* Write the configuration. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GUSBCFG, usbcfg.d32);

    /* Always return success. */
    return (SUCCESS);

} /* usb_stm32f407_set_current_mode */

/*
 * usb_stm32f407_get_current_mode
 * @usb_device: USB device instance.
 * Returns current USB mode.
 */
uint32_t usb_stm32f407_get_current_mode(USB_STM32F407_HANDLE *usb_device)
{
    /* Return current USB mode. */
    return (OS_READ_REG32(&usb_device->regs.GREGS->GINTSTS ) & 0x1);

} /* usb_stm32f407_get_current_mode */

/*
 * usb_stm32f407_is_device_mode
 * @usb_device: USB device instance.
 * Checks if we are in device mode.
 */
uint8_t usb_stm32f407_is_device_mode(USB_STM32F407_HANDLE *usb_device)
{
    /* Return if we are in device mode. */
    return (usb_stm32f407_get_current_mode(usb_device) != USB_STM32F407_HOST_MODE);

} /* usb_stm32f407_is_device_mode */


/*
 * usb_stm32f407_is_device_mode
 * @usb_device: USB device instance.
 * Checks if we are in host mode.
 */
uint8_t usb_stm32f407_is_host_mode(USB_STM32F407_HANDLE *usb_device)
{
    /* Return if we are in host mode. */
    return (usb_stm32f407_get_current_mode(usb_device) == USB_STM32F407_HOST_MODE);

} /* usb_stm32f407_is_host_mode */

/*
 * usb_stm32f407_read_otg_interrupt
 * @usb_device: USB device instance.
 * Returns OTG interrupt register.
 */
uint32_t usb_stm32f407_read_otg_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    /* Return OTG interrupt register. */
    return (OS_READ_REG32 (&usb_device->regs.GREGS->GOTGINT));

} /* usb_stm32f407_read_otg_interrupt */

/*
 * usb_stm32f407_read_core_interrupt
 * @usb_device: USB device instance.
 * Returns core interrupt register.
 */
uint32_t usb_stm32f407_read_core_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    uint32_t status = 0;

    /* Get interrupt register. */
    status = OS_READ_REG32(&usb_device->regs.GREGS->GINTSTS);

    /* Mask the interrupts. */
    status &= OS_READ_REG32(&usb_device->regs.GREGS->GINTMSK);

    /* Return interrupt status. */
    return (status);

} /* usb_stm32f407_read_core_interrupt */

#endif /* CONFIG_USB */
