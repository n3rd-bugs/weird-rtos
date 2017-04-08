/*
 * usb_function_stm32f407_interrupt.c
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
#include <usb.h>

#ifdef USB_FUNCTION
#include <string.h>

/* Internal functions. */
/* USB interrupt handlers. */
#ifdef STM32F407_USB_VBUS_SENSING_ENABLED
static uint32_t usb_function_stm32f407_session_request_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_otg_interrupt(USB_STM32F407_HANDLE *);
#endif
static uint32_t usb_function_stm32f407_resume_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_suspend_interrupt(USB_STM32F407_HANDLE *);

static uint32_t usb_function_stm32f407_in_ep_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_out_ep_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_sof_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_rx_status_queue_level_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_reset_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_enum_done_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_iso_in_incomplete_interrupt(USB_STM32F407_HANDLE *);
static uint32_t usb_function_stm32f407_iso_out_incomplete_interrupt(USB_STM32F407_HANDLE *);

static uint32_t usb_function_stm32f407_write_empty_tx_fifo(USB_STM32F407_HANDLE *, uint32_t);
static uint32_t usb_function_stm32f407_read_in_ep_flag(USB_STM32F407_HANDLE *, uint8_t);

/*
 * usb_function_stm32f407_interrupt_handler
 * @usb_device: USB device instance.
 * This function will handle USB function interrupts.
 */
uint32_t usb_function_stm32f407_interrupt_handler(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS gintr_status;
    uint32_t status = SUCCESS;

    /* Ensure we are in device mode. */
    if (usb_stm32f407_is_device_mode(pdev))
    {
        /* Get core interrupt status. */
        gintr_status.d32 = usb_stm32f407_read_core_interrupt(pdev);

        /* Check if we do have an interrupt. */
        if (!gintr_status.d32)
        {
            /* Should never happen. */
            OS_ASSERT(TRUE);

            return 0;
        }

        /* If we have an OUT endpoint interrupt. */
        if (gintr_status.b.outepintr)
        {
            /* Handle OUT endpoint interrupt. */
            status |= usb_function_stm32f407_out_ep_interrupt(pdev);
        }

        /* If we have an IN endpoint interrupt. */
        if (gintr_status.b.inepint)
        {
            /* Handle IN endpoint interrupt. */
            status |= usb_function_stm32f407_in_ep_interrupt(pdev);
        }

        /* If we have a mode mismatch interrupt. */
        if (gintr_status.b.modemismatch)
        {
            USB_STM32F407_GINTSTS gintsts;

            /* Clear interrupt. */
            gintsts.d32 = 0;
            gintsts.b.modemismatch = 1;
            OS_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);
        }

        /* If we have a wake up interrupt. */
        if (gintr_status.b.wkupintr)
        {
            /* Handle resume interrupt. */
            status |= usb_function_stm32f407_resume_interrupt(pdev);
        }

        /* If we have a suspend interrupt. */
        if (gintr_status.b.usbsuspend)
        {
            /* Handle suspend interrupt. */
            status |= usb_function_stm32f407_suspend_interrupt(pdev);
        }

        /* If we have a SOF interrupt. */
        if (gintr_status.b.sofintr)
        {
            /* Handle SOF interrupt. */
            status |= usb_function_stm32f407_sof_interrupt(pdev);
        }

        /* If we have a RX queue level interrupt. */
        if (gintr_status.b.rxstsqlvl)
        {
            /* Handle RX queue level interrupt. */
            status |= usb_function_stm32f407_rx_status_queue_level_interrupt(pdev);
        }

        /* If we have a reset interrupt. */
        if (gintr_status.b.usbreset)
        {
            /* Handle reset interrupt. */
            status |= usb_function_stm32f407_reset_interrupt(pdev);
        }

        /* If we have a ENUM done interrupt. */
        if (gintr_status.b.enumdone)
        {
            /* Handle ENUM done interrupt. */
            status |= usb_function_stm32f407_enum_done_interrupt(pdev);
        }

        /* If we have a ISO IN incomplete interrupt. */
        if (gintr_status.b.incomplisoin)
        {
            /* Handle ISO IN incomplete interrupt. */
            status |= usb_function_stm32f407_iso_in_incomplete_interrupt(pdev);
        }

        /* If we have a ISO OUT incomplete interrupt. */
        if (gintr_status.b.incomplisoout)
        {
            /* Handle ISO OUT incomplete interrupt. */
            status |= usb_function_stm32f407_iso_out_incomplete_interrupt(pdev);
        }

#ifdef STM32F407_USB_VBUS_SENSING_ENABLED
        /* If we have a session request interrupt. */
        if (gintr_status.b.sessreqintr)
        {
            /* Handle session request interrupt. */
            status |= usb_function_stm32f407_session_request_interrupt(pdev);
        }

        /* If we have an OTG interrupt. */
        if (gintr_status.b.otgintr)
        {
            /* Handle OTG interrupt. */
            status |= usb_function_stm32f407_otg_interrupt(pdev);
        }
#endif
    }

    /* Return status to caller. */
    return (status);

} /* usb_function_stm32f407_interrupt_handler */

#ifdef STM32F407_USB_HS_DEDICATED_EP1_ENABLED
/*
 * usb_function_stm32f407_ep1_out_interrupt
 * @usb_device: USB device instance.
 * This function will handle endpoint 1 OUT interrupt.
 */
uint32_t usb_function_stm32f407_ep1_out_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_DOEPINT doepint;
    USB_STM32F407_DEPXFRSIZ deptsiz;

    doepint.d32 = OS_READ_REG32(&pdev->regs.OUTEP_REGS[1]->DOEPINT);
    doepint.d32&= OS_READ_REG32(&pdev->regs.DREGS->DOUTEP1MSK);

    /* If transfer is complete. */
    if (doepint.b.xfercompl)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_OUT_EP_INTR(1, xfercompl);

        /* If we are using DMA. */
        if (pdev->cfg.dma_enable == 1)
        {
            deptsiz.d32 = OS_READ_REG32(&(pdev->regs.OUTEP_REGS[1]->DOEPTSIZ));

            /* TODO: Handle more than one single MPS size packet. */
            pdev->dev.out_ep[1].xfer_count = pdev->dev.out_ep[1].maxpacket - deptsiz.b.xfersize;
        }

        /* Inform upper layer that data is ready. */
        usb_function_interrupt_cb->data_out_stage(pdev, 1);

    }

    /* If endpoint is disabled.  */
    if (doepint.b.epdisabled)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_OUT_EP_INTR(1, epdisabled);
    }

    /* If we have AHB error. */
    if (doepint.b.ahberr)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_OUT_EP_INTR(1, ahberr);
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep1_out_interrupt */

/*
 * usb_function_stm32f407_ep1_in_interrupt
 * @usb_device: USB device instance.
 * This function will handle endpoint 1 IN interrupt.
 */
uint32_t usb_function_stm32f407_ep1_in_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_DIEPINT diepint;
    uint32_t fifoemptymsk, msk, emp;

    msk = OS_READ_REG32(&pdev->regs.DREGS->DINEP1MSK);
    emp = OS_READ_REG32(&pdev->regs.DREGS->DIEPEMPMSK);

    msk |= ((emp >> 1) & 0x1) << 7;
    diepint.d32  = OS_READ_REG32(&pdev->regs.INEP_REGS[1]->DIEPINT) & msk;

    if (diepint.b.xfercompl)
    {
        fifoemptymsk = 0x1 << 1;
        OS_MASK_REG32(&pdev->regs.DREGS->DIEPEMPMSK, fifoemptymsk, 0);

        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, xfercompl);

        /* Inform upper layer that data is needed. */
        usb_function_interrupt_cb->data_in_stage(pdev , 1);
    }

    /* If we have AHB error. */
    if (diepint.b.ahberr)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, ahberr);
    }

    /* If we have endpoint disabled interrupt. */
    if (diepint.b.epdisabled)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, epdisabled);
    }

    /* If we have timeout interrupt. */
    if (diepint.b.timeout)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, timeout);
    }

    /* If we have IN token interrupt. */
    if (diepint.b.intktxfemp)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, intktxfemp);
    }

    /* If we have IN token interrupt. */
    if (diepint.b.intknepmis)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, intknepmis);
    }

    /* If we have IN endpoint interrupt. */
    if (diepint.b.inepnakeff)
    {
        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, inepnakeff);
    }

    /* If we have empty interrupt. */
    if (diepint.b.emptyintr)
    {
        /* Handle empty TX FIFO. */
        usb_function_stm32f407_write_empty_tx_fifo(pdev, 1);

        /* Clear the bit in DOEPINT for this interrupt. */
        CLEAR_IN_EP_INTR(1, emptyintr);
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_ep1_in_interrupt */
#endif

#ifdef STM32F407_USB_VBUS_SENSING_ENABLED
/*
 * usb_function_stm32f407_session_request_interrupt
 * @usb_device: USB device instance.
 * This function will handle session request interrupt.
 */
static uint32_t usb_function_stm32f407_session_request_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS gintsts;

    /* Tell upper layer that we are connected. */
    usb_function_interrupt_cb->connected(pdev);

    /* Clear interrupt flag. */
    gintsts.d32 = 0;
    gintsts.b.sessreqintr = 1;
    OS_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_session_request_interrupt */

/*
 * usb_function_stm32f407_session_request_interrupt
 * @usb_device: USB device instance.
 * This function will handle OTG interrupt.
 */
static uint32_t usb_function_stm32f407_otg_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GOTGINT gotgint;

    /* Get OTG interrupt status. */
    gotgint.d32 = OS_READ_REG32(&pdev->regs.GREGS->GOTGINT);

    /* If we need to send disconnect. */
    if (gotgint.b.sesenddet)
    {
        /* Tell upper layer that we are disconnected. */
        usb_function_interrupt_cb->disconnected(pdev);
    }

    /* Clear interrupt flag. */
    OS_WRITE_REG32(&pdev->regs.GREGS->GOTGINT, gotgint.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_otg_interrupt */
#endif

/*
 * usb_function_stm32f407_resume_interrupt
 * @usb_device: USB device instance.
 * This function will handle resume interrupt.
 */
static uint32_t usb_function_stm32f407_resume_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS   gintsts;
    USB_STM32F407_DCTL      devctl;
    USB_STM32F407_PCGCCTL   power;

    /* If we are using low power mode. */
    if(pdev->cfg.low_power)
    {
        /* Un-gate USB core clock. */
        memcpy(&power.d32, (uint32_t *)pdev->regs.PCGCCTL, sizeof(uint32_t));
        power.b.gatehclk = 0;
        power.b.stoppclk = 0;
        OS_WRITE_REG32(pdev->regs.PCGCCTL, power.d32);
    }

    /* Clear the remote wake up signaling */
    devctl.d32 = 0;
    devctl.b.rmtwkupsig = 1;
    OS_MASK_REG32(&pdev->regs.DREGS->DCTL, devctl.d32, 0);

    /* Transfer call to upper layer to handle this event. */
    usb_function_interrupt_cb->reset (pdev);

    /* Clear interrupt flag. */
    gintsts.d32 = 0;
    gintsts.b.wkupintr = 1;
    OS_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_resume_interrupt */

/*
 * usb_function_stm32f407_suspend_interrupt
 * @usb_device: USB device instance.
 * This function will handle suspend interrupt.
 */
static uint32_t usb_function_stm32f407_suspend_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS  gintsts;
    USB_STM32F407_PCGCCTL  power;
    USB_STM32F407_DSTS     dsts;

    /* Transfer call to upper layer to handle this event. */
    usb_function_interrupt_cb->suspend(pdev);

    /* Get device status. */
    dsts.d32 = OS_READ_REG32(&pdev->regs.DREGS->DSTS);

    /* Clear interrupt flag. */
    gintsts.d32 = 0;
    gintsts.b.usbsuspend = 1;
    OS_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* If we are in low power mode. */
    if((pdev->cfg.low_power) && (dsts.b.suspsts == 1))
    {
        /* Switch-off the clocks. */
        power.d32 = 0;
        power.b.stoppclk = 1;
        OS_MASK_REG32(pdev->regs.PCGCCTL, 0, power.d32);

        power.b.gatehclk = 1;
        OS_MASK_REG32(pdev->regs.PCGCCTL, 0, power.d32);

        /* Request to enter sleep mode after exit from current interrupt. */
        SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk);
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_suspend_interrupt */

/*
 * usb_function_stm32f407_in_ep_interrupt
 * @usb_device: USB device instance.
 * This function will handle IN endpoint interrupt.
 */
static uint32_t usb_function_stm32f407_in_ep_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_DIEPINT diepint;
    uint32_t ep_intr;
    uint32_t epnum = 0;
    uint32_t fifoemptymsk;
    diepint.d32 = 0;

    /* Get IN endpoint interrupt register. */
    ep_intr = usb_function_stm32f407_ep_get_device_all_in_interrupt(pdev);

    /* While we have an interrupt to process. */
    while (ep_intr)
    {
        /* If we need to handle interrupt for this endpoint. */
        if (ep_intr & 0x1)
        {
            /* Get endpoint interrupt status. */
            diepint.d32 = usb_function_stm32f407_read_in_ep_flag(pdev, (uint8_t)epnum);

            /* If transfer was complete. */
            if (diepint.b.xfercompl)
            {
                fifoemptymsk = (uint32_t)(0x1 << epnum);
                OS_MASK_REG32(&pdev->regs.DREGS->DIEPEMPMSK, fifoemptymsk, 0);
                CLEAR_IN_EP_INTR(epnum, xfercompl);

                /* Transfer call to upper layer so that data can be sent. */
                usb_function_interrupt_cb->data_in_stage(pdev, (uint8_t)epnum);

                /* If we are using DMA. */
                if (pdev->cfg.dma_enable == 1)
                {
                    /* If this is a control endpoint. */
                    if((epnum == 0) && (pdev->device.state == USB_STM32F407_EP0_STATUS_IN))
                    {
                        /* Prepare control endpoint so more data can be sent. */
                        usb_function_stm32f407_ep0_start_out(pdev);
                    }
                }
            }

            /* If we have AHB error. */
            if (diepint.b.ahberr)
            {
                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, ahberr);
            }

            /* If a timeout has accrued. */
            if (diepint.b.timeout)
            {
                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, timeout);
            }

            /* If we need to handle IN token interrupt. */
            if (diepint.b.intktxfemp)
            {
                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, intktxfemp);
            }

            /* If we need to handle IN token interrupt. */
            if (diepint.b.intknepmis)
            {
                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, intknepmis);
            }

            /* If we need to handle IN token interrupt. */
            if (diepint.b.inepnakeff)
            {
                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, inepnakeff);
            }

            /* If endpoint was disabled. */
            if (diepint.b.epdisabled)
            {
                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, epdisabled);
            }

            /* If FIFO is empty. */
            if (diepint.b.emptyintr)
            {
                /* Handle TX FIFO empty event. */
                usb_function_stm32f407_write_empty_tx_fifo(pdev , epnum);

                /* Clear interrupt flag. */
                CLEAR_IN_EP_INTR(epnum, emptyintr);
            }
        }

        /* Process next interrupt. */
        epnum++;
        ep_intr >>= 1;
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_in_ep_interrupt */

/*
 * usb_function_stm32f407_out_ep_interrupt
 * @usb_device: USB device instance.
 * This function will handle OUT endpoint interrupt.
 */
static uint32_t usb_function_stm32f407_out_ep_interrupt(USB_STM32F407_HANDLE *pdev)
{
    uint32_t ep_intr;
    USB_STM32F407_DOEPINT doepint;
    USB_STM32F407_DEPXFRSIZ deptsiz;
    uint32_t epnum = 0;

    doepint.d32 = 0;

    /* Read in the device interrupt status. */
    ep_intr = usb_function_stm32f407_ep_get_device_all_out_interrupt(pdev);

    /* While we have a pending interrupt. */
    while (ep_intr)
    {
        /* If interrupt flag is set for this endpoint. */
        if (ep_intr & 0x1)
        {
            /* Get the out endpoint interrupt status. */
            doepint.d32 = usb_function_stm32f407_ep_get_device_out_interrupt(pdev, (uint8_t)epnum);

            /* If transfer is complete. */
            if (doepint.b.xfercompl)
            {
                /* Clear interrupt flag. */
                CLEAR_OUT_EP_INTR(epnum, xfercompl);

                /* If we are using DMA. */
                if (pdev->cfg.dma_enable == 1)
                {
                    deptsiz.d32 = OS_READ_REG32(&(pdev->regs.OUTEP_REGS[epnum]->DOEPTSIZ));

                    /* TODO: Handle more than one single MPS size packet. */
                    pdev->device.out_ep[epnum].xfer_count = pdev->device.out_ep[epnum].maxpacket - \
                    deptsiz.b.xfersize;
                }

                /* Transfer call to USB function driver. */
                usb_function_interrupt_cb->data_out_stage(pdev, (uint8_t)epnum);

                /* If we are using DMA. */
                if (pdev->cfg.dma_enable == 1)
                {
                    if((epnum == 0) && (pdev->device.state == USB_STM32F407_EP0_STATUS_OUT))
                    {
                        /* Setup control endpoint to receive more packets. */
                        usb_function_stm32f407_ep0_start_out(pdev);
                    }
                }
            }

            /* If endpoint was disabled.  */
            if (doepint.b.epdisabled)
            {
                /* Clear interrupt flag. */
                CLEAR_OUT_EP_INTR(epnum, epdisabled);
            }

            /* If we have AHB error. */
            if (doepint.b.ahberr)
            {
                /* Clear interrupt flag. */
                CLEAR_OUT_EP_INTR(epnum, ahberr);
            }

            /* Setup phase done. */
            if (doepint.b.setup)
            {
                /* Tell upper layer that setup stage is done. */
                usb_function_interrupt_cb->setup_stage(pdev);

                /* Clear interrupt flag. */
                CLEAR_OUT_EP_INTR(epnum, setup);
            }
        }

        /* Process next endpoint. */
        epnum++;
        ep_intr >>= 1;
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_out_ep_interrupt */

/*
 * usb_function_stm32f407_sof_interrupt
 * @usb_device: USB device instance.
 * This function will handle SOF interrupt.
 */
static uint32_t usb_function_stm32f407_sof_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS GINTSTS;

    /* Transfer call to function driver. */
    usb_function_interrupt_cb->sof(pdev);

    /* Clear interrupt flag. */
    GINTSTS.d32 = 0;
    GINTSTS.b.sofintr = 1;
    OS_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, GINTSTS.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_sof_interrupt */

/*
 * usb_function_stm32f407_rx_status_queue_level_interrupt
 * @usb_device: USB device instance.
 * This function will handle RX status queue level interrupt.
 */
static uint32_t usb_function_stm32f407_rx_status_queue_level_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTMSK int_mask;
    USB_STM32F407_DRXSTS status;
    USB_ENDPOINT *ep;

    /* Disable the RX status queue level interrupt. */
    int_mask.d32 = 0;
    int_mask.b.rxstsqlvl = 1;
    OS_MASK_REG32(&pdev->regs.GREGS->GINTMSK, int_mask.d32, 0);

    /* Get the status from the top of the FIFO. */
    status.d32 = OS_READ_REG32(&pdev->regs.GREGS->GRXSTSP);

    /* Get the endpoint for which this interrupt was generated. */
    ep = &pdev->device.out_ep[status.b.epnum];

    /* Handle the interrupt. */
    switch (status.b.pktsts)
    {

    /* Data update. */
    case STS_DATA_UPDT:
        if (status.b.bcnt)
        {
            usb_stm32f407_read_packet(pdev,ep->xfer_buff, status.b.bcnt);
            ep->xfer_buff += status.b.bcnt;
            ep->xfer_count += status.b.bcnt;
        }

        break;

    /* Status update. */
    case STS_SETUP_UPDT:

        /* Copy the setup packet received in FIFO into the setup buffer in RAM. */
        usb_stm32f407_read_packet(pdev , pdev->device.setup_packet, 8);
        ep->xfer_count += status.b.bcnt;

        break;

    case STS_GOUT_NAK:
    case STS_XFER_COMP:
    case STS_SETUP_COMP:
    default:

        break;

    }

    /* Enable RX status queue level interrupt. */
    OS_MASK_REG32(&pdev->regs.GREGS->GINTMSK, 0, int_mask.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_rx_status_queue_level_interrupt */

/*
 * usb_function_stm32f407_reset_interrupt
 * @usb_device: USB device instance.
 * This function will handle reset interrupt.
 */
static uint32_t usb_function_stm32f407_reset_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_DAINT     daintmsk;
    USB_STM32F407_DOEPMSK   doepmsk;
    USB_STM32F407_DIEPMSK   diepmsk;
    USB_STM32F407_DCFG      dcfg;
    USB_STM32F407_DCTL      dctl;
    USB_STM32F407_GINTSTS   gintsts;
    uint32_t i;

    dctl.d32 = 0;
    daintmsk.d32 = 0;
    doepmsk.d32 = 0;
    diepmsk.d32 = 0;
    dcfg.d32 = 0;
    gintsts.d32 = 0;

    /* Clear the remote wake-up signaling. */
    dctl.b.rmtwkupsig = 1;
    OS_MASK_REG32(&pdev->regs.DREGS->DCTL, dctl.d32, 0);

    /* Flush the TX FIFO. */
    usb_stm32f407_flush_tx_fifo(pdev,  0);

    /* Reset all endpoints. */
    for (i = 0; i < pdev->cfg.dev_endpoints ; i++)
    {
        OS_WRITE_REG32(&pdev->regs.INEP_REGS[i]->DIEPINT, 0xFF);
        OS_WRITE_REG32(&pdev->regs.OUTEP_REGS[i]->DOEPINT, 0xFF);
    }

    /* Initialize IN endpoint interrupts. */
    OS_WRITE_REG32(&pdev->regs.DREGS->DAINT, 0xFFFFFFFF);

    daintmsk.ep.in = 1;
    daintmsk.ep.out = 1;
    OS_WRITE_REG32(&pdev->regs.DREGS->DAINTMSK, daintmsk.d32);

    doepmsk.b.setup = 1;
    doepmsk.b.xfercompl = 1;
    doepmsk.b.ahberr = 1;
    doepmsk.b.epdisabled = 1;
    OS_WRITE_REG32(&pdev->regs.DREGS->DOEPMSK, doepmsk.d32);

#ifdef STM32F407_USB_HS_DEDICATED_EP1_ENABLED
    OS_WRITE_REG32(&pdev->regs.DREGS->DOUTEP1MSK, doepmsk.d32);
#endif

    diepmsk.b.xfercompl = 1;
    diepmsk.b.timeout = 1;
    diepmsk.b.epdisabled = 1;
    diepmsk.b.ahberr = 1;
    diepmsk.b.intknepmis = 1;
    OS_WRITE_REG32(&pdev->regs.DREGS->DIEPMSK, diepmsk.d32);

#ifdef STM32F407_USB_HS_DEDICATED_EP1_ENABLED
    OS_WRITE_REG32(&pdev->regs.DREGS->DINEP1MSK, diepmsk.d32);
#endif

    /* Reset device address. */
    dcfg.d32 = OS_READ_REG32(&pdev->regs.DREGS->DCFG);
    dcfg.b.devaddr = 0;
    OS_WRITE_REG32(&pdev->regs.DREGS->DCFG, dcfg.d32);

    /* Setup EP0 to receive setup packets. */
    usb_function_stm32f407_ep0_start_out(pdev);

    /* Clear interrupt flag. */
    gintsts.d32 = 0;
    gintsts.b.usbreset = 1;
    OS_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* Reset internal state machine. */
    usb_function_interrupt_cb->reset(pdev);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_reset_interrupt */

/*
 * usb_function_stm32f407_enum_done_interrupt
 * @usb_device: USB device instance.
 * This function will handle ENUM done interrupt.
 */
static uint32_t usb_function_stm32f407_enum_done_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS gintsts;
    USB_STM32F407_GUSBCFG gusbcfg;

    /* Deactivate the control endpoint. */
    usb_function_stm32f407_ep0_deactivate(pdev);

    /* Set USB turn-around time based on device speed and PHY interface. */
    gusbcfg.d32 = OS_READ_REG32(&pdev->regs.GREGS->GUSBCFG);

    /* If we are in HS mode. */
    if (usb_function_stm32f407_get_device_speed(pdev) == HIGH)
    {
        pdev->cfg.speed            = USB_STM32F407_SPEED_HIGH;
        pdev->cfg.max_packet_size              = USB_STM32F407_HS_MAX_PACKET_SIZE ;
        gusbcfg.b.usbtrdtim = 9;
    }

    /* We are in FS mode. */
    else
    {
        pdev->cfg.speed            = USB_STM32F407_SPEED_FULL;
        pdev->cfg.max_packet_size              = USB_STM32F407_FS_MAX_PACKET_SIZE ;
        gusbcfg.b.usbtrdtim = 5;
    }

    /* Update USB configuration. */
    OS_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, gusbcfg.d32);

    /* Clear interrupt flag. */
    gintsts.d32 = 0;
    gintsts.b.enumdone = 1;
    OS_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_enum_done_interrupt */

/*
 * usb_function_stm32f407_iso_in_incomplete_interrupt
 * @usb_device: USB device instance.
 * This function will handle ISO IN incomplete interrupt.
 */
static uint32_t usb_function_stm32f407_iso_in_incomplete_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS gintsts;

    gintsts.d32 = 0;

    /* Transfer call to USB function driver. */
    usb_function_interrupt_cb->iso_in_incomplete(pdev);

    /* Clear interrupt flag. */
    gintsts.b.incomplisoin = 1;
    OS_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_iso_in_incomplete_interrupt */

/*
 * usb_function_stm32f407_iso_out_incomplete_interrupt
 * @usb_device: USB device instance.
 * This function will handle ISO OUT incomplete interrupt.
 */
static uint32_t usb_function_stm32f407_iso_out_incomplete_interrupt(USB_STM32F407_HANDLE *pdev)
{
    USB_STM32F407_GINTSTS gintsts;

    gintsts.d32 = 0;

    /* Transfer call to USB function driver. */
    usb_function_interrupt_cb->iso_out_incomplete(pdev);

    /* Clear interrupt flag. */
    gintsts.b.incomplisoout = 1;
    OS_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_iso_out_incomplete_interrupt */

/*
 * usb_function_stm32f407_write_empty_tx_fifo
 * @usb_device: USB device instance.
 * @number: Endpoint number.
 * This function will load the TX FIFO for next packet.
 */
static uint32_t usb_function_stm32f407_write_empty_tx_fifo(USB_STM32F407_HANDLE *pdev, uint32_t number)
{
    USB_STM32F407_DTXFSTS txstatus;
    USB_ENDPOINT *ep;
    uint32_t len = 0;
    uint32_t len32b;
    txstatus.d32 = 0;

    /* Get the endpoint. */
    ep = &pdev->device.in_ep[number];

    /* Calculate the length. */
    len = ep->xfer_len - ep->xfer_count;

    /* If we have to send more data than max packet size. */
    if (len > ep->maxpacket)
    {
        /* Just send maximum length of data that can be sent. */
        len = ep->maxpacket;
    }

    /* Calculate the number of words to be sent. */
    len32b = (len + 3) / 4;

    /* Get the endpoint status. */
    txstatus.d32 = OS_READ_REG32(&pdev->regs.INEP_REGS[number]->DTXFSTS);

    /* Write the TX FIFO. */
    while ((txstatus.b.txfspcavail > len32b) &&
           (ep->xfer_count < ep->xfer_len) &&
           (ep->xfer_len != 0))
    {
        /* Calculate length needed to be sent. */
        len = ep->xfer_len - ep->xfer_count;

        /* If we have to send more data than max packet size. */
        if (len > ep->maxpacket)
        {
            /* Just send maximum length of data that can be sent. */
            len = ep->maxpacket;
        }

        /* Calculate the number of words to be sent. */
        len32b = (len + 3) / 4;

        /* Write the packet on FIFO. */
        usb_stm32f407_write_packet(pdev, ep->xfer_buff, (uint8_t)number, len);

        ep->xfer_buff  += len;
        ep->xfer_count += len;

        txstatus.d32 = OS_READ_REG32(&pdev->regs.INEP_REGS[number]->DTXFSTS);
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_function_stm32f407_write_empty_tx_fifo */

/*
 * usb_function_stm32f407_read_in_ep_flag
 * @usb_device: USB device instance.
 * @number: Endpoint number.
 * This function will return IN endpoint flags.
 */
static uint32_t usb_function_stm32f407_read_in_ep_flag(USB_STM32F407_HANDLE *pdev, uint8_t number)
{
    uint32_t value, msk, emp;

    /* Get the IN endpoint flags. */
    msk = OS_READ_REG32(&pdev->regs.DREGS->DIEPMSK);
    emp = OS_READ_REG32(&pdev->regs.DREGS->DIEPEMPMSK);
    msk |= ((emp >> number) & 0x1) << 7;
    value = OS_READ_REG32(&pdev->regs.INEP_REGS[number]->DIEPINT) & msk;

    /* Return IN endpoint flags. */
    return (value);

} /* usb_function_stm32f407_read_in_ep_flag */

#endif /* USB_FUNCTION */
