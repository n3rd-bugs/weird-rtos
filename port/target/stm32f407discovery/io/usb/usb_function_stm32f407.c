/*
 * usb_function_stm32f407.c
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
#include <string.h>

#ifdef USB_FUNCTION

#ifdef STM32F407_USB_CDC_ACM
/* Needs to be word aligned. */
USB_FUN_CDC_ACM_DEV stm32f407_usb_cdc_device __attribute__ ((aligned (0x10)));
#endif
#ifdef STM32F407_USB_CDC_ACM_PPP
PPP     stm32f407_usb_cdc_ppp_instance;
NET_DEV stm32f407_usb_cdc_net_device;
#endif

ISR_FUN usb_otg_interrupt()
{
    OS_ISR_ENTER();

#ifdef STM32F407_USB_CDC_ACM
    usb_function_stm32f407_interrupt_handler (&stm32f407_usb_cdc_device.usb);
#endif

    OS_ISR_EXIT();
} /* usb_otg_interrupt */

/*
 * usb_function_stm32f407_init
 * This function will initialize USB function device for STM32F407.
 */
void usb_function_stm32f407_init()
{
#ifdef STM32F407_USB_CDC_ACM
#ifdef STM32F407_USB_CDC_DEBUG
    extern FD debug_usart_fd;
#endif

    memset(&stm32f407_usb_cdc_device, 0, sizeof(USB_FUN_CDC_ACM_DEV));

    /* Initialize BSP. */
    usb_stm32f407_hw_initilaize((USB_STM32F407_HANDLE *)&stm32f407_usb_cdc_device);

    /* Initialize USB function device. */
    usb_function_init((USB_STM32F407_HANDLE *)&stm32f407_usb_cdc_device, &usb_fun_cdc_acm_cb);

    /* Initialize and register a console for this device. */
    /* For now we only support one CDC function console. */
    stm32f407_usb_cdc_device.cdc_console.console.fs.name = "cdcacmf0";
    usb_cdc_console_register(&stm32f407_usb_cdc_device.cdc_console, &stm32f407_usb_cdc_device);

    /* Enable interrupts. */
    usb_stm32f407_enable_interrupt((USB_STM32F407_HANDLE *)&stm32f407_usb_cdc_device);

#ifdef STM32F407_USB_CDC_DEBUG
    /* Connect this descriptor to the UART file descriptor. */
    fs_connect((FD)&USB_CDC_Device.cdc_console, debug_usart_fd);
#elif defined(STM32F407_USB_CDC_ACM_PPP)

    /* Register USB CDC file descriptor with PPP. */
    ppp_register_fd(&stm32f407_usb_cdc_ppp_instance, (FD)&stm32f407_usb_cdc_device.cdc_console, TRUE);

    /* Register USB CDC file descriptor with the networking stack. */
    net_register_fd(&stm32f407_usb_cdc_net_device, (FD)&stm32f407_usb_cdc_device.cdc_console);
#endif
#endif

} /* usb_function_stm32f407_init */

/*
 * usb_function_stm32f407_init
 * @usb_device: USB device instance.
 * This function will initialize USB device registers for STM32F407.
 */
uint32_t usb_function_stm32f407_core_initialize_device(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_DEPCTL    depctl;
    USB_STM32F407_DCFG      dcfg;
    USB_STM32F407_FSIZ      nptxfifosize;
    USB_STM32F407_FSIZ      txfifosize;
    USB_STM32F407_DIEPMSK   msk;
    USB_STM32F407_DTHRCTL   dthrctl;
    uint32_t i;

    depctl.d32 = 0;
    dcfg.d32 = 0;
    nptxfifosize.d32 = 0;
    txfifosize.d32 = 0;
    msk.d32 = 0;

    /* Restart the physical clock. */
    OS_WRITE_REG32(usb_device->regs.PCGCCTL, 0);

    /* Device configuration register. */
    dcfg.d32 = OS_READ_REG32( &usb_device->regs.DREGS->DCFG);
    dcfg.b.perfrint = DCFG_FRAME_INTERVAL_80;
    OS_WRITE_REG32( &usb_device->regs.DREGS->DCFG, dcfg.d32 );

#if defined(STM32F407_USB_FS_CORE)

    /* Initialize device speed. */
    usb_function_stm32f407_initialize_speed(usb_device , USB_STM32F407_SPEED_PARAM_FULL);

    /* Set RX FIFO size. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRXFSIZ, RX_FIFO_FS_SIZE);

    /* Initialize EP0 in TX mode. */
    nptxfifosize.b.depth     = TX0_FIFO_FS_SIZE;
    nptxfifosize.b.startaddr = RX_FIFO_FS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF0_HNPTXFSIZ, nptxfifosize.d32 );

    /* Initialize EP1 in TX mode. */
    txfifosize.b.startaddr = (uint16_t)(nptxfifosize.b.startaddr + nptxfifosize.b.depth);
    txfifosize.b.depth = TX1_FIFO_FS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[0], txfifosize.d32 );

    /* Initialize EP2 in TX mode. */
    txfifosize.b.startaddr = (uint16_t)(txfifosize.b.startaddr + txfifosize.b.depth);
    txfifosize.b.depth = TX2_FIFO_FS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[1], txfifosize.d32 );

    /* Initialize EP3 in TX mode. */
    txfifosize.b.startaddr = (uint16_t)(txfifosize.b.startaddr + txfifosize.b.depth);
    txfifosize.b.depth = TX3_FIFO_FS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[2], txfifosize.d32 );

#elif defined(STM32F407_USB_HS_CORE)

    /* Initialize device speed. */

    /* If we have ULPI physical. */
    if(usb_device->cfg.phy_itface  == STM32F407_USB_ULPI_PHY)
    {
        /* Initialize in HS mode. */
        usb_function_stm32f407_initialize_speed(usb_device , USB_STM32F407_SPEED_PARAM_HIGH);
    }
    else
    {
        /* Initialize in FS mode. */
        usb_function_stm32f407_initialize_speed(usb_device , USB_STM32F407_SPEED_PARAM_HIGH_IN_FULL);
    }

    /* Set RX FIFO size. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GRXFSIZ, RX_FIFO_HS_SIZE);

    /* Initialize EP0 in TX mode. */
    nptxfifosize.b.depth     = TX0_FIFO_HS_SIZE;
    nptxfifosize.b.startaddr = RX_FIFO_HS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF0_HNPTXFSIZ, nptxfifosize.d32 );

    /* Initialize EP1 in TX mode. */
    txfifosize.b.startaddr = nptxfifosize.b.startaddr + nptxfifosize.b.depth;
    txfifosize.b.depth = TX1_FIFO_HS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[0], txfifosize.d32 );

    /* Initialize EP2 in TX mode. */
    txfifosize.b.startaddr += txfifosize.b.depth;
    txfifosize.b.depth = TX2_FIFO_HS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[1], txfifosize.d32 );

    /* Initialize EP3 in TX mode. */
    txfifosize.b.startaddr += txfifosize.b.depth;
    txfifosize.b.depth = TX3_FIFO_HS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[2], txfifosize.d32 );

    /* Initialize EP4 in TX mode. */
    txfifosize.b.startaddr += txfifosize.b.depth;
    txfifosize.b.depth = TX4_FIFO_HS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[3], txfifosize.d32 );

    /* Initialize EP5 in TX mode. */
    txfifosize.b.startaddr += txfifosize.b.depth;
    txfifosize.b.depth = TX5_FIFO_HS_SIZE;
    OS_WRITE_REG32( &usb_device->regs.GREGS->DIEPTXF[4], txfifosize.d32 );

#endif

    /* Flush the FIFOs. */
    usb_stm32f407_flush_tx_fifo(usb_device, 0x10);
    usb_stm32f407_flush_rx_fifo(usb_device);

    /* Clear all pending device interrupts. */
    OS_WRITE_REG32( &usb_device->regs.DREGS->DIEPMSK, 0 );
    OS_WRITE_REG32( &usb_device->regs.DREGS->DOEPMSK, 0 );
    OS_WRITE_REG32( &usb_device->regs.DREGS->DAINT, 0xFFFFFFFF );
    OS_WRITE_REG32( &usb_device->regs.DREGS->DAINTMSK, 0 );

    for (i = 0; i < usb_device->cfg.dev_endpoints; i++)
    {
        /* Initialize IN endpoints. */
        depctl.d32 = OS_READ_REG32(&usb_device->regs.INEP_REGS[i]->DIEPCTL);
        if (depctl.b.epena)
        {
            depctl.d32 = 0;
            depctl.b.epdis = 1;
            depctl.b.snak = 1;
        }
        else
        {
            depctl.d32 = 0;
        }

        OS_WRITE_REG32( &usb_device->regs.INEP_REGS[i]->DIEPCTL, depctl.d32);
        OS_WRITE_REG32( &usb_device->regs.INEP_REGS[i]->DIEPTSIZ, 0);
        OS_WRITE_REG32( &usb_device->regs.INEP_REGS[i]->DIEPINT, 0xFF);

        /* Initialize OUT endpoint. */
        depctl.d32 = OS_READ_REG32(&usb_device->regs.OUTEP_REGS[i]->DOEPCTL);
        if (depctl.b.epena)
        {
            depctl.d32 = 0;
            depctl.b.epdis = 1;
            depctl.b.snak = 1;
        }
        else
        {
            depctl.d32 = 0;
        }

        OS_WRITE_REG32( &usb_device->regs.OUTEP_REGS[i]->DOEPCTL, depctl.d32);
        OS_WRITE_REG32( &usb_device->regs.OUTEP_REGS[i]->DOEPTSIZ, 0);
        OS_WRITE_REG32( &usb_device->regs.OUTEP_REGS[i]->DOEPINT, 0xFF);
    }

    msk.d32 = 0;
    msk.b.txfifoundrn = 1;
    OS_MASK_REG32(&usb_device->regs.DREGS->DIEPMSK, msk.d32, msk.d32);

    if (usb_device->cfg.dma_enable == 1)
    {
        dthrctl.d32 = 0;
        dthrctl.b.non_iso_thr_en = 1;
        dthrctl.b.iso_thr_en = 1;
        dthrctl.b.tx_thr_len = 64;
        dthrctl.b.rx_thr_en = 1;
        dthrctl.b.rx_thr_len = 64;

        OS_WRITE_REG32(&usb_device->regs.DREGS->DTHRCTL, dthrctl.d32);
    }

    /* Enable USB interrupts. */
    usb_function_stm32f407_enable_interrupts(usb_device);

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_core_initialize_device */

/*
 * usb_function_stm32f407_initialize_speed
 * @usb_device: USB device instance.
 * @speed: USB speed to be configured.
 * This function will return the current device speed.
 */
void usb_function_stm32f407_initialize_speed(USB_STM32F407_HANDLE *usb_device, uint8_t speed)
{
    USB_STM32F407_DCFG   dcfg;

    /* Read configuration. */
    dcfg.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DCFG);

    /* Initialize configuration. */
    dcfg.b.devspd = speed & MASK_N_BITS(2);

    /* Update configuration. */
    OS_WRITE_REG32(&usb_device->regs.DREGS->DCFG, dcfg.d32);

} /* usb_function_stm32f407_initialize_speed */

/*
 * usb_function_stm32f407_get_device_speed
 * @usb_device: USB device instance.
 * This function will return the current device speed.
 */
enum USB_STM32F407_SPEED usb_function_stm32f407_get_device_speed(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_DSTS  dsts;
    enum USB_STM32F407_SPEED speed = UNKNOWN;

    /* Get device status register. */
    dsts.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DSTS);

    /* Calculate the device speed. */
    switch (dsts.b.enumspd)
    {
    case (DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ):
        speed = HIGH;
        break;

    case (DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ):
    case (DSTS_ENUMSPD_FS_PHY_48MHZ):
        speed = FULL;
        break;

    case (DSTS_ENUMSPD_LS_PHY_6MHZ):
        speed = LOW;
        break;
    }

    /* Return the device speed. */
    return (speed);

} /* usb_function_stm32f407_get_device_speed */

/*
 * usb_function_stm32f407_ep0_deactivate
 * @usb_device: USB device instance.
 * This function will activate the endpoint 0.
 */
uint32_t usb_function_stm32f407_ep0_deactivate(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_DSTS      dsts;
    USB_STM32F407_DEPCTL    diepctl;
    USB_STM32F407_DCTL      dctl;

    dctl.d32 = 0;

    /* Read the device status and endpoint 0 control registers */
    dsts.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DSTS);
    diepctl.d32 = OS_READ_REG32(&usb_device->regs.INEP_REGS[0]->DIEPCTL);

    /* Set the MPS of the IN endpoint based on the enumeration speed. */
    switch (dsts.b.enumspd)
    {
    case (DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ):
    case (DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ):
    case (DSTS_ENUMSPD_FS_PHY_48MHZ):
        diepctl.b.mps = DEP0CTL_MPS_64;
        break;
    case (DSTS_ENUMSPD_LS_PHY_6MHZ):
        diepctl.b.mps = DEP0CTL_MPS_8;
        break;
    }

    OS_WRITE_REG32(&usb_device->regs.INEP_REGS[0]->DIEPCTL, diepctl.d32);
    dctl.b.cgnpinnak = 1;
    OS_MASK_REG32(&usb_device->regs.DREGS->DCTL, dctl.d32, dctl.d32);

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep0_deactivate */

/*
 * usb_function_stm32f407_ep_activate
 * @usb_device: USB device instance.
 * @ep: Endpoint to activate.
 * This function will activate an endpoint.
 */
uint32_t usb_function_stm32f407_ep_activate(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL    depctl;
    USB_STM32F407_DAINT     daintmsk;
    volatile uint32_t       *addr;

    depctl.d32 = 0;
    daintmsk.d32 = 0;

    /* Read DEPCTL register. */
    if (ep->is_in == 1)
    {
        addr = &usb_device->regs.INEP_REGS[ep->num]->DIEPCTL;
        daintmsk.ep.in = (uint16_t)(1 << ep->num);
    }
    else
    {
        addr = &usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL;
        daintmsk.ep.out = (uint16_t)(1 << ep->num);
    }

    /* If the EP is already active don't change the EP control register. */
    depctl.d32 = OS_READ_REG32(addr);
    if (!depctl.b.usbactep)
    {
        depctl.b.mps        = ep->maxpacket & MASK_N_BITS(11);
        depctl.b.eptype     = ep->type & MASK_N_BITS(2);
        depctl.b.txfnum     = ep->tx_fifo_num & MASK_N_BITS(4);
        depctl.b.setd0pid   = 1;
        depctl.b.usbactep   = 1;
        OS_WRITE_REG32(addr, depctl.d32);
    }

    /* Enable the Interrupt for this EP */
#ifdef STM32F407_USB_HS_DEDICATED_EP1_ENABLED
    if((ep->num == 1)&&(usb_device->cfg.coreID == USB_OTG_HS_CORE_ID))
    {
        OS_MASK_REG32(&usb_device->regs.DREGS->DEACHMSK, 0, daintmsk.d32);
    }
    else
#endif
    {
        OS_MASK_REG32(&usb_device->regs.DREGS->DAINTMSK, 0, daintmsk.d32);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep_activate */

/*
 * usb_function_stm32f407_ep_deactivate
 * @usb_device: USB device instance.
 * This function will return the current device speed.
 */
uint32_t usb_function_stm32f407_ep_deactivate(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL  depctl;
    USB_STM32F407_DAINT  daintmsk;
    volatile uint32_t *addr;

    depctl.d32 = 0;
    daintmsk.d32 = 0;

    /* Read the DEPCTL register. */
    if (ep->is_in == 1)
    {
        addr = &usb_device->regs.INEP_REGS[ep->num]->DIEPCTL;
        daintmsk.ep.in = (uint16_t)(1 << ep->num);
    }
    else
    {
        addr = &usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL;
        daintmsk.ep.out = (uint16_t)(1 << ep->num);
    }

    depctl.b.usbactep = 0;
    OS_WRITE_REG32(addr, depctl.d32);

    /* Disable the interrupt for this EP. */
#ifdef STM32F407_USB_HS_DEDICATED_EP1_ENABLED
    if ((ep->num == 1) && (usb_device->cfg.coreID == USB_OTG_HS_CORE_ID))
    {
        OS_MASK_REG32(&usb_device->regs.DREGS->DEACHMSK, daintmsk.d32, 0);
    }
    else
#endif
    {
        OS_MASK_REG32(&usb_device->regs.DREGS->DAINTMSK, daintmsk.d32, 0);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep_deactivate */

/*
 * usb_function_stm32f407_ep_start_transfer
 * @usb_device: USB device instance.
 * @ep: Endpoint to setup.
 * This function will setup an endpoint for transfer.
 */
uint32_t usb_function_stm32f407_ep_start_transfer(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL        depctl;
    USB_STM32F407_DEPXFRSIZ     deptsiz;
    USB_STM32F407_DSTS          dsts;
    uint32_t fifoemptymsk = 0;

    depctl.d32 = 0;
    deptsiz.d32 = 0;

    /* If this is an IN endpoint. */
    if (ep->is_in == 1)
    {
        /* Read control and size registers. */
        depctl.d32  = OS_READ_REG32(&(usb_device->regs.INEP_REGS[ep->num]->DIEPCTL));
        deptsiz.d32 = OS_READ_REG32(&(usb_device->regs.INEP_REGS[ep->num]->DIEPTSIZ));

        /* If this is a zero length packet. */
        if (ep->xfer_len == 0)
        {
            deptsiz.b.xfersize = 0;
            deptsiz.b.pktcnt = 1;
        }
        else
        {
            /* Program the transfer size and packet count. */
            deptsiz.b.xfersize = ep->xfer_len & MASK_N_BITS(19);
            deptsiz.b.pktcnt = ((ep->xfer_len - 1 + ep->maxpacket) / ep->maxpacket) & MASK_N_BITS(10);

            if (ep->type == EP_TYPE_ISOC)
            {
                deptsiz.b.mc = 1;
            }
        }

        /* Write the EP size. */
        OS_WRITE_REG32(&usb_device->regs.INEP_REGS[ep->num]->DIEPTSIZ, deptsiz.d32);

        /* Setup endpoint interrupt. */
        if (usb_device->cfg.dma_enable == 1)
        {
            OS_WRITE_REG32(&usb_device->regs.INEP_REGS[ep->num]->DIEPDMA, ep->dma_addr);
        }
        else
        {
            if (ep->type != EP_TYPE_ISOC)
            {
                /* Enable the TX FIFO empty interrupt for this EP. */
                if (ep->xfer_len > 0)
                {
                    fifoemptymsk = (uint16_t)(1 << ep->num);
                    OS_MASK_REG32(&usb_device->regs.DREGS->DIEPEMPMSK, 0, fifoemptymsk);
                }
            }
        }

        /* If this is ISOC type endpoint. */
        if (ep->type == EP_TYPE_ISOC)
        {
            dsts.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DSTS);

            if (((dsts.b.soffn) & 0x1) == 0)
            {
                depctl.b.setd1pid = 1;
            }
            else
            {
                depctl.b.setd0pid = 1;
            }
        }

        /* Enable endpoint and push data in the FIFO. */
        depctl.b.cnak = 1;
        depctl.b.epena = 1;
        OS_WRITE_REG32(&usb_device->regs.INEP_REGS[ep->num]->DIEPCTL, depctl.d32);

        if (ep->type == EP_TYPE_ISOC)
        {
            usb_stm32f407_write_packet(usb_device, ep->xfer_buff, ep->num, ep->xfer_len);
        }
    }
    else
    {
        /* This is a OUT endpoint. */
        /* Read control and size registers. */
        depctl.d32  = OS_READ_REG32(&(usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL));
        deptsiz.d32 = OS_READ_REG32(&(usb_device->regs.OUTEP_REGS[ep->num]->DOEPTSIZ));

        /* Program the transfer size and packet count. */
        if (ep->xfer_len == 0)
        {
            deptsiz.b.xfersize = ep->maxpacket & MASK_N_BITS(19);
            deptsiz.b.pktcnt = 1;
        }
        else
        {
            deptsiz.b.xfersize = (deptsiz.b.pktcnt * ep->maxpacket) & MASK_N_BITS(19) ;
            deptsiz.b.pktcnt = ((ep->xfer_len + (ep->maxpacket - 1)) / ep->maxpacket) & MASK_N_BITS(10);
        }

        /* Write the EP size. */
        OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPTSIZ, deptsiz.d32);

        if (usb_device->cfg.dma_enable == 1)
        {
            OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPDMA, ep->dma_addr);
        }

        /* If this is ISOC type endpoint. */
        if (ep->type == EP_TYPE_ISOC)
        {
            if (ep->even_odd_frame)
            {
                depctl.b.setd1pid = 1;
            }
            else
            {
                depctl.b.setd0pid = 1;
            }
        }

        /* Enable this endpoint. */
        depctl.b.cnak = 1;
        depctl.b.epena = 1;
        OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL, depctl.d32);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep_start_transfer */

/*
 * usb_function_stm32f407_ep_start_transfer
 * @usb_device: USB device instance.
 * @ep: Control endpoint.
 * This function will setup endpoint0 for transfer.
 */
uint32_t usb_function_stm32f407_ep0_start_transfer(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL      depctl;
    USB_STM32F407_DEP0XFRSIZ  deptsiz;
    USB_STM32F407_INEPREGS    *in_regs;
    uint32_t fifoemptymsk = 0;

    depctl.d32   = 0;
    deptsiz.d32  = 0;

    /* If this is a IN endpoint. */
    if (ep->is_in == 1)
    {
        /* Get IN endpoint register. */
        in_regs = usb_device->regs.INEP_REGS[0];

        /* Get control and size registers. */
        depctl.d32  = OS_READ_REG32(&in_regs->DIEPCTL);
        deptsiz.d32 = OS_READ_REG32(&in_regs->DIEPTSIZ);

        /* If this is a zero length packet. */
        if (ep->xfer_len == 0)
        {
            deptsiz.b.xfersize = 0;
            deptsiz.b.pktcnt = 1;
        }
        else
        {
            /* Calculate the packet size and count. */
            if (ep->xfer_len > ep->maxpacket)
            {
                ep->xfer_len = ep->maxpacket;
                deptsiz.b.xfersize = ep->maxpacket & MASK_N_BITS(7);
                deptsiz.b.pktcnt = ((ep->xfer_len % ep->maxpacket) + 1) & MASK_N_BITS(2);
            }
            else
            {
                deptsiz.b.xfersize = ep->xfer_len & MASK_N_BITS(7);
                deptsiz.b.pktcnt = 1;
            }
        }

        /* Write the endpoint size. */
        OS_WRITE_REG32(&in_regs->DIEPTSIZ, deptsiz.d32);

        if (usb_device->cfg.dma_enable == 1)
        {
            OS_WRITE_REG32(&usb_device->regs.INEP_REGS[ep->num]->DIEPDMA, ep->dma_addr);
        }

        /* Enable endpoint and write data to be sent. */
        depctl.b.cnak = 1;
        depctl.b.epena = 1;
        OS_WRITE_REG32(&in_regs->DIEPCTL, depctl.d32);

        if (usb_device->cfg.dma_enable == 0)
        {
            /* Enable the Tx FIFO Empty Interrupt for this EP */
            if (ep->xfer_len > 0)
            {
                fifoemptymsk |= (uint32_t)(1 << ep->num);
                OS_MASK_REG32(&usb_device->regs.DREGS->DIEPEMPMSK, 0, fifoemptymsk);
            }
        }
    }
    else
    {
        /* If this is a OUT endpoint. */
        /* Read control and size registers. */
        depctl.d32  = OS_READ_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL);
        deptsiz.d32 = OS_READ_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPTSIZ);

        /* Program the transfer size and packet. */
        if (ep->xfer_len == 0)
        {
            deptsiz.b.xfersize = ep->maxpacket & MASK_N_BITS(7);
            deptsiz.b.pktcnt = 1;
        }
        else
        {
            ep->xfer_len = ep->maxpacket;
            deptsiz.b.xfersize = ep->maxpacket & MASK_N_BITS(7);
            deptsiz.b.pktcnt = 1;
        }

        /* Write the endpoint size. */
        OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPTSIZ, deptsiz.d32);

        if (usb_device->cfg.dma_enable == 1)
        {
            OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[ep->num]->DOEPDMA, ep->dma_addr);
        }

        /* Enable this endpoint. */
        depctl.b.cnak = 1;
        depctl.b.epena = 1;
        OS_WRITE_REG32 (&(usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL), depctl.d32);

    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep0_start_transfer */

/*
 * usb_function_stm32f407_ep_set_stall
 * @usb_device: USB function device instance.
 * @ep: Endpoint needed to be stalled.
 * This function will stall a given endpoint.
 */
uint32_t usb_function_stm32f407_ep_set_stall(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL depctl;
    volatile uint32_t *depctl_addr;

    depctl.d32 = 0;

    /* If this is an IN endpoint. */
    if (ep->is_in == 1)
    {
        /* Read the endpoint configuration. */
        depctl_addr = &(usb_device->regs.INEP_REGS[ep->num]->DIEPCTL);
        depctl.d32 = OS_READ_REG32(depctl_addr);

        /* If this endpoint is enabled. */
        if (depctl.b.epena)
        {
            /* Disable this endpoint. */
            depctl.b.epdis = 1;
        }

        /* Set stall. */
        depctl.b.stall = 1;

        /* Update endpoint configuration. */
        OS_WRITE_REG32(depctl_addr, depctl.d32);
    }

    /* This is an OUT endpoint. */
    else
    {
        /* Read the endpoint configuration. */
        depctl_addr = &(usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL);
        depctl.d32 = OS_READ_REG32(depctl_addr);

        /* Set stall. */
        depctl.b.stall = 1;

        /* Update endpoint configuration. */
        OS_WRITE_REG32(depctl_addr, depctl.d32);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep_set_stall */

/*
 * usb_function_stm32f407_ep_clear_stall
 * @usb_device: USB function device instance.
 * @ep: Endpoint for which stall needed to be cleared.
 * Clears stall for a given endpoint.
 */
uint32_t usb_function_stm32f407_ep_clear_stall(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL  depctl;
    volatile uint32_t *depctl_addr;

    depctl.d32 = 0;

    /* Get the endpoint configuration. */
    if (ep->is_in == 1)
    {
        depctl_addr = &(usb_device->regs.INEP_REGS[ep->num]->DIEPCTL);
    }
    else
    {
        depctl_addr = &(usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL);
    }
    depctl.d32 = OS_READ_REG32(depctl_addr);

    /* Clear the stall for this endpoint. */
    depctl.b.stall = 0;

    /* If this is interrupt or bulk endpoint. */
    if ((ep->type == EP_TYPE_INTR) || (ep->type == EP_TYPE_BULK))
    {
        depctl.b.setd0pid = 1;
    }

    /* Update endpoint configuration. */
    OS_WRITE_REG32(depctl_addr, depctl.d32);

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep_clear_stall */

/*
 * usb_function_stm32f407_stop_device
 * @usb_device: USB function device instance.
 * @ep: Endpoint for which status is needed.
 * Returns status for a given endpoint.
 */
uint32_t usb_function_stm32f407_ep_get_status(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep)
{
    USB_STM32F407_DEPCTL depctl;
    volatile uint32_t *depctl_addr;
    uint32_t status = USB_STM32F407_EP_TX_DIS;

    depctl.d32 = 0;

    /* If this is an IN endpoint. */
    if (ep->is_in == 1)
    {
        /* Get endpoint configuration. */
        depctl_addr = &(usb_device->regs.INEP_REGS[ep->num]->DIEPCTL);
        depctl.d32 = OS_READ_REG32(depctl_addr);

        /* Get endpoint status. */
        if (depctl.b.stall == 1)
        {
            status = USB_STM32F407_EP_TX_STALL;
        }
        else if (depctl.b.naksts == 1)
        {
            status = USB_STM32F407_EP_TX_NAK;
        }
        else
        {
            status = USB_STM32F407_EP_TX_VALID;
        }
    }


    /* This is an OUT endpoint. */
    else
    {
        /* Get endpoint configuration. */
        depctl_addr = &(usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL);
        depctl.d32 = OS_READ_REG32(depctl_addr);

        /* Get endpoint status. */
        if (depctl.b.stall == 1)
        {
            status = USB_STM32F407_EP_RX_STALL;
        }
        else if (depctl.b.naksts == 1)
        {
            status = USB_STM32F407_EP_RX_NAK;
        }
        else
        {
            status = USB_STM32F407_EP_RX_VALID;
        }
    }

    /* Return endpoint status. */
    return (status);

} /* usb_function_stm32f407_ep_get_status */

/*
 * usb_function_stm32f407_ep_set_status
 * @usb_device: USB function device instance.
 * @ep: Endpoint for which status is needed to be updated.
 * Updates status for a given endpoint.
 */
void usb_function_stm32f407_ep_set_status(USB_STM32F407_HANDLE *usb_device, USB_ENDPOINT *ep, uint32_t status)
{
    USB_STM32F407_DEPCTL  depctl;
    volatile uint32_t *depctl_addr;

    depctl.d32 = 0;

    /* Process for IN endpoint */
    if (ep->is_in == 1)
    {
        /* Get endpoint configuration. */
        depctl_addr = &(usb_device->regs.INEP_REGS[ep->num]->DIEPCTL);
        depctl.d32 = OS_READ_REG32(depctl_addr);

        /* Configure endpoint status. */
        if (status == USB_STM32F407_EP_TX_STALL)
        {
            usb_function_stm32f407_ep_set_stall(usb_device, ep);
            return;
        }
        else if (status == USB_STM32F407_EP_TX_NAK)
        {
            depctl.b.snak = 1;
        }
        else if (status == USB_STM32F407_EP_TX_VALID)
        {
            /* If we are stalled. */
            if (depctl.b.stall == 1)
            {
                ep->even_odd_frame = 0;
                usb_function_stm32f407_ep_clear_stall(usb_device, ep);
                return;
            }

            depctl.b.cnak = 1;
            depctl.b.usbactep = 1;
            depctl.b.epena = 1;
        }
        else if (status == USB_STM32F407_EP_TX_DIS)
        {
            depctl.b.usbactep = 0;
        }
    }
    else /* Process for OUT endpoint */
    {
        /* Get endpoint configuration. */
        depctl_addr = &(usb_device->regs.OUTEP_REGS[ep->num]->DOEPCTL);
        depctl.d32 = OS_READ_REG32(depctl_addr);

        /* Configure endpoint status. */
        if (status == USB_STM32F407_EP_RX_STALL)
        {
            depctl.b.stall = 1;
        }
        else if (status == USB_STM32F407_EP_RX_NAK)
        {
            depctl.b.snak = 1;
        }
        else if (status == USB_STM32F407_EP_RX_VALID)
        {
            /* If we are stalled. */
            if (depctl.b.stall == 1)
            {
                ep->even_odd_frame = 0;
                usb_function_stm32f407_ep_clear_stall(usb_device, ep);
                return;
            }

            depctl.b.cnak = 1;
            depctl.b.usbactep = 1;
            depctl.b.epena = 1;
        }
        else if (status == USB_STM32F407_EP_RX_DIS)
        {
            depctl.b.usbactep = 0;
        }
    }

    /* Update endpoint status. */
    OS_WRITE_REG32(depctl_addr, depctl.d32);

} /* usb_function_stm32f407_ep_set_status */

/*
 * usb_function_stm32f407_ep_get_device_all_out_interrupt
 * @usb_device: USB function device instance.
 * Returns the all OUT endpoint interrupt status.
 */
uint32_t usb_function_stm32f407_ep_get_device_all_out_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    uint32_t value;

    /* Read the interrupt status. */
    value  = OS_READ_REG32(&usb_device->regs.DREGS->DAINT);
    value &= OS_READ_REG32(&usb_device->regs.DREGS->DAINTMSK);

    /* Return all OUT endpoint(s) interrupt status. */
    return ((value & 0xffff0000) >> 16);

} /* usb_function_stm32f407_ep_get_device_all_out_interrupt */

/*
 * usb_function_stm32f407_ep_get_device_out_interrupt
 * @usb_device: USB function device instance.
 * Returns the OUT endpoint interrupt status.
 */
uint32_t usb_function_stm32f407_ep_get_device_out_interrupt(USB_STM32F407_HANDLE *usb_device , uint8_t epnum)
{
    uint32_t value;

    /* Read the interrupt status. */
    value  = OS_READ_REG32(&usb_device->regs.OUTEP_REGS[epnum]->DOEPINT);
    value &= OS_READ_REG32(&usb_device->regs.DREGS->DOEPMSK);

    /* Return OUT endpoint interrupt status. */
    return (value);

} /* usb_function_stm32f407_ep_get_device_out_interrupt */

/*
 * usb_function_stm32f407_ep_get_device_all_in_interrupt
 * @usb_device: USB function device instance.
 * Returns the all IN endpoint interrupt status.
 */
uint32_t usb_function_stm32f407_ep_get_device_all_in_interrupt(USB_STM32F407_HANDLE *usb_device)
{
    uint32_t value;

    /* Read the interrupt status. */
    value = OS_READ_REG32(&usb_device->regs.DREGS->DAINT);
    value &= OS_READ_REG32(&usb_device->regs.DREGS->DAINTMSK);

    /* Return all IN endpoint(s) interrupt status. */
    return (value & 0xffff);

} /* usb_function_stm32f407_ep_get_device_all_in_interrupt */

/*
 * usb_function_stm32f407_ep0_start_out
 * @usb_device: USB function device instance.
 * Setup control endpoint to receive setup packets.
 */
void usb_function_stm32f407_ep0_start_out(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_DEP0XFRSIZ doeptsize0;

    /* Initialize size configuration. */
    doeptsize0.d32 = 0;
    doeptsize0.b.supcnt = 3;
    doeptsize0.b.pktcnt = 1;
    doeptsize0.b.xfersize = 8 * 3;
    OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[0]->DOEPTSIZ, doeptsize0.d32);

    /* If we are using DMA. */
    if (usb_device->cfg.dma_enable == 1)
    {
        USB_STM32F407_DEPCTL doepctl;

        doepctl.d32 = 0;
        OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[0]->DOEPDMA, (uint32_t)&usb_device->device.setup_packet);

        /* Enable this endpoint. */
        doepctl.d32 = OS_READ_REG32(&usb_device->regs.OUTEP_REGS[0]->DOEPCTL);
        doepctl.b.epena = 1;
        doepctl.d32 = 0x80008000;
        OS_WRITE_REG32( &usb_device->regs.OUTEP_REGS[0]->DOEPCTL, doepctl.d32);
    }

} /* usb_function_stm32f407_ep0_start_out */

/*
 * usb_function_stm32f407_activate_remote_wakeup
 * @usb_device: USB function device instance.
 * Activate remote wake up on USB device.
 */
void usb_function_stm32f407_activate_remote_wakeup(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_DCTL      dctl;
    USB_STM32F407_DSTS      dsts;
    USB_STM32F407_PCGCCTL   power;

    /* If remote wake up is active. */
    if (usb_device->device.remote_wakeup)
    {
        /* Get device status. */
        dsts.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DSTS);

        /* If we are suspended. */
        if(dsts.b.suspsts == 1)
        {
            /* If we are low power device. */
            if(usb_device->cfg.low_power)
            {
                /* Un-gate USB core clock. */
                power.d32 = OS_READ_REG32(&usb_device->regs.PCGCCTL);
                power.b.gatehclk = 0;
                power.b.stoppclk = 0;
                OS_WRITE_REG32(usb_device->regs.PCGCCTL, power.d32);
            }

            /* Activate remote wake up signaling. */
            dctl.d32 = 0;
            dctl.b.rmtwkupsig = 1;
            OS_MASK_REG32(&usb_device->regs.DREGS->DCTL, 0, dctl.d32);

            sleep(10);

            OS_MASK_REG32(&usb_device->regs.DREGS->DCTL, dctl.d32, 0);
        }
    }

} /* usb_function_stm32f407_activate_remote_wakeup */

/*
 * usb_function_stm32f407_ungate_clock
 * @usb_device: USB function device instance.
 * Activate USB core clock.
 */
void usb_function_stm32f407_ungate_clock(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_DSTS      dsts;
    USB_STM32F407_PCGCCTL   power;

    /* If we are low power device. */
    if(usb_device->cfg.low_power)
    {
        /* Get device configuration. */
        dsts.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DSTS);

        /* If we are suspended. */
        if(dsts.b.suspsts == 1)
        {
            /* Un-gate USB core clock. */
            power.d32 = OS_READ_REG32(&usb_device->regs.PCGCCTL);
            power.b.gatehclk = 0;
            power.b.stoppclk = 0;
            OS_WRITE_REG32(usb_device->regs.PCGCCTL, power.d32);

        }
    }

} /* usb_function_stm32f407_ungate_clock */

/*
 * usb_function_stm32f407_stop_device
 * @usb_device: USB function device instance.
 * Stop a USB device instance.
 */
void usb_function_stm32f407_stop_device(USB_STM32F407_HANDLE *usb_device)
{
    uint32_t i;

    /* Update device status. */
    usb_device->device.status = 1;

    /* Deinitialize all endpoints. */
    for (i = 0; i < usb_device->cfg.dev_endpoints ; i++)
    {
        OS_WRITE_REG32(&usb_device->regs.INEP_REGS[i]->DIEPINT, 0xFF);
        OS_WRITE_REG32(&usb_device->regs.OUTEP_REGS[i]->DOEPINT, 0xFF);
    }

    /* Deinitialize the device configuration. */
    OS_WRITE_REG32(&usb_device->regs.DREGS->DIEPMSK, 0);
    OS_WRITE_REG32(&usb_device->regs.DREGS->DOEPMSK, 0);
    OS_WRITE_REG32(&usb_device->regs.DREGS->DAINTMSK, 0);
    OS_WRITE_REG32(&usb_device->regs.DREGS->DAINT, 0xFFFFFFFF);

    /* Flush the FIFOs. */
    usb_stm32f407_flush_rx_fifo(usb_device);
    usb_stm32f407_flush_tx_fifo(usb_device, 0x10);

} /* usb_function_stm32f407_stop_device */

/*
 * usb_function_stm32f407_init
 * @usb_device: USB function device instance.
 * This function will initialize USB function interrupts.
 */
uint32_t usb_function_stm32f407_enable_interrupts(USB_STM32F407_HANDLE *usb_device)
{
    USB_STM32F407_GINTMSK  intmsk;

    intmsk.d32 = 0;

    /* Disable all interrupts. */
    OS_WRITE_REG32(&usb_device->regs.GREGS->GINTMSK, 0);

    /* Enable the common USB interrupts. */
    usb_stm32f407_enable_common_interrupt(usb_device);

    /* If we are using DMA. */
    if (usb_device->cfg.dma_enable == 0)
    {
        intmsk.b.rxstsqlvl = 1;
    }

    /* Enable device mode interrupts. */
    intmsk.b.usbsuspend = 1;
    intmsk.b.usbreset   = 1;
    intmsk.b.enumdone   = 1;
    intmsk.b.inepintr   = 1;
    intmsk.b.outepintr  = 1;
    intmsk.b.sofintr    = 1;
    intmsk.b.incomplisoin   = 1;
    intmsk.b.incomplisoout  = 1;

#ifdef STM32F407_USB_VBUS_SENSING_ENABLED
    intmsk.b.sessreqintr    = 1;
    intmsk.b.otgintr        = 1;
#endif

    /* Update interrupt configuration. */
    OS_MASK_REG32( &usb_device->regs.GREGS->GINTMSK, intmsk.d32, intmsk.d32);

    /* Always return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_enable_interrupts */

/*
 * usb_function_stm32f407_connect
 * @usb_device: USB device instance.
 * This will connect the USB device on the USB bus.
 */
void usb_function_stm32f407_connect(USB_STM32F407_HANDLE *usb_device)
{
#ifndef STM32F407_USB_OTG_MODE
    USB_STM32F407_DCTL dctl;

    /* Get current configuration. */
    dctl.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DCTL);

    /* Update configuration. */
    dctl.b.sftdiscon = 0;

    /* Connect the device on USB bus. */
    OS_WRITE_REG32(&usb_device->regs.DREGS->DCTL, dctl.d32);
#endif

} /* usb_function_stm32f407_connect */

/*
 * usb_function_stm32f407_disconnect
 * @usb_device: USB device instance.
 * This will disconnect the USB device from USB bus.
 */
void usb_function_stm32f407_disconnect(USB_STM32F407_HANDLE *usb_device)
{
#ifndef STM32F407_USB_OTG_MODE
    USB_STM32F407_DCTL  dctl;

    /* Get current configuration. */
    dctl.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DCTL);

    /* Update configuration. */
    dctl.b.sftdiscon = 1;

    /* Disconnect this device from USB bus. */
    OS_WRITE_REG32(&usb_device->regs.DREGS->DCTL, dctl.d32);
#endif

} /* usb_function_stm32f407_disconnect */

#endif /* USB_FUNCTION */
