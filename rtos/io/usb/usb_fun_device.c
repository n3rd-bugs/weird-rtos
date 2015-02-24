/*
 * usb_fun_device.c
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

#ifdef USB_FUNCTION

/*
 * usb_fun_device_init
 * @usb_device: USB device instance.
 * Initializes a USB function device.
 */
void usb_fun_device_init(USB_STM32F407_HANDLE *usb_device)
{
    uint8_t i;
    USB_ENDPOINT *ep;

    /* Initialize USB core parameters. */
    usb_stm32f407_select_core(usb_device);

    /* Initialize device status. */
    usb_device->device.status = USB_FUN_STATE_DEFAULT;
    usb_device->device.address = 0;

    /* Initialize IN endpoints. */
    for (i = 0; i < usb_device->cfg.dev_endpoints ; i++)
    {
        /* The the endpoint. */
        ep = &usb_device->device.in_ep[i];

        /* Initialize this endpoint. */
        ep->is_in = 1;
        ep->num = i;
        ep->tx_fifo_num = i;

        /* Treat this endpoint as control endpoint until we use it. */
        ep->type = EP_TYPE_CTRL;
        ep->maxpacket = USB_STM32F407_MAX_EP0_SIZE;
        ep->xfer_buff = 0;
        ep->xfer_len = 0;
    }

    /* Initialize OUT endpoints. */
    for (i = 0; i < usb_device->cfg.dev_endpoints; i++)
    {
        /* The the endpoint. */
        ep = &usb_device->device.out_ep[i];

        /* Initialize this endpoint. */
        ep->is_in = 0;
        ep->num = i;
        ep->tx_fifo_num = i;

        /* Treat this endpoint as control endpoint until we use it. */
        ep->type = EP_TYPE_CTRL;
        ep->maxpacket = USB_STM32F407_MAX_EP0_SIZE;
        ep->xfer_buff = 0;
        ep->xfer_len = 0;
    }

    /* Disable USB interrupts. */
    usb_stm32f407_disable_global_interrupt(usb_device);

    /* Initialize USB core. */
    usb_stm32f407_core_initialize(usb_device);

    /* Initialize device mode for this device. */
    usb_stm32f407_set_current_mode(usb_device, USB_STM32F407_DEVICE_MODE);

    /* Initialize USB device core. */
    usb_function_stm32f407_core_initialize_device(usb_device);

    /* Enable USB interrupts. */
    usb_stm32f407_enable_global_interrupt(usb_device);

} /* usb_fun_device_init */

/*
 * usb_fun_endpoint_open
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * @max_size: Maximum endpoint size.
 * @type: Type of endpoint needed to be initialized.
 * Configures an endpoint, also activates it on controller.
 */
uint32_t usb_fun_endpoint_open(USB_STM32F407_HANDLE *usb_device, uint8_t address, uint16_t max_size, uint8_t type)
{
    USB_ENDPOINT *ep;

    /* Pick a required endpoint. */
    if (address & 0x80)
    {
        /* This is an IN endpoint. */
        ep = &usb_device->device.in_ep[address & 0x7F];
    }
    else
    {
        /* This is an OUT endpoint. */
        ep = &usb_device->device.out_ep[address & 0x7F];
    }

    /* Extract the endpoint number. */
    ep->num = (address & 0x7F);

    /* Initialize this endpoint. */
    ep->is_in = ((0x80 & address) != 0);
    ep->maxpacket = max_size;
    ep->type = type;

    if (ep->is_in)
    {
        /* Assign TX FIFO number. */
        ep->tx_fifo_num = ep->num;
    }

    /* If we are initializing a bulk endpoint. */
    if (type == EP_TYPE_BULK)
    {
        ep->data_pid_start = 0;
    }

    /* Activate this endpoint. */
    usb_function_stm32f407_ep_activate(usb_device, ep);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_open */

/*
 * usb_fun_endpoint_close
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * Closes an endpoint, also deactivate it on controller.
 */
uint32_t usb_fun_endpoint_close(USB_STM32F407_HANDLE *usb_device, uint8_t address)
{
    USB_ENDPOINT *ep;

    /* Pick the required endpoint. */
    if (address & 0x80)
    {
        /* This is an IN endpoint. */
        ep = &usb_device->device.in_ep[address & 0x7F];
    }
    else
    {
        /* This is an OUT endpoint. */
        ep = &usb_device->device.out_ep[address & 0x7F];
    }

    /* Initialize endpoint number. */
    ep->num = address & 0x7F;
    ep->is_in = ((0x80 & address) != 0);

    /* Deactivate the endpoint on the controller. */
    usb_function_stm32f407_ep_deactivate(usb_device , ep );

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_close */

/*
 * usb_fun_endpoint_prepare_rx
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * @buffer: Buffer in which data is needed to be received.
 * @length: Length of buffer, or number of bytes to receive in this buffer.
 * This will prepare an endpoint for receiving data.
 */
uint32_t usb_fun_endpoint_prepare_rx(USB_STM32F407_HANDLE *usb_device, uint8_t address, uint8_t *buffer, uint32_t length)
{
    USB_ENDPOINT *ep;

    /* Get the endpoint for which we need to start RX. */
    ep = &usb_device->device.out_ep[address & 0x7F];

    /* Setup this endpoint for RX. */
    ep->xfer_buff = buffer;
    ep->xfer_len = length;
    ep->xfer_count = 0;
    ep->is_in = 0;
    ep->num = address & 0x7F;

    /* If we are using DMA. */
    if (usb_device->cfg.dma_enable == 1)
    {
        /* Assign same buffer for DMA RX. */
        ep->dma_addr = (uint32_t)buffer;
    }

    /* If this is a control endpoint. */
    if (ep->num == 0)
    {
        /* Start RX on control endpoint. */
        usb_function_stm32f407_ep0_start_transfer(usb_device, ep);
    }
    else
    {
        /* Start RX on given endpoint. */
        usb_function_stm32f407_ep_start_transfer(usb_device, ep );
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_prepare_rx */

/*
 * usb_fun_endpoint_tx
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * @buffer: Buffer from which data is needed to be sent.
 * @length: Number of bytes needed to be sent from this buffer.
 * This will start transferring data on an endpoint.
 */
uint32_t usb_fun_endpoint_tx(USB_STM32F407_HANDLE *usb_device, uint8_t address, uint8_t *buffer, uint32_t length)
{
    USB_ENDPOINT *ep;

    /* Get the endpoint. */
    ep = &usb_device->device.in_ep[address & 0x7F];

    /* Setup and start the Transfer */
    ep->is_in = 1;
    ep->num = address & 0x7F;
    ep->xfer_buff = buffer;
    ep->xfer_count = 0;
    ep->xfer_len = length;

    /* If we are using DMA. */
    if (usb_device->cfg.dma_enable == 1)
    {
        /* Assign same buffer for DMA TX. */
        ep->dma_addr = (uint32_t)buffer;
    }

    /* If this is a control endpoint. */
    if ( ep->num == 0 )
    {
        /* Start TX on control endpoint. */
        usb_function_stm32f407_ep0_start_transfer(usb_device, ep);
    }
    else
    {
        /* Start TX on given endpoint. */
        usb_function_stm32f407_ep_start_transfer(usb_device, ep );
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_tx */

/*
 * usb_fun_endpoint_stall
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * This will set stall on an endpoint.
 */
uint32_t usb_fun_endpoint_stall(USB_STM32F407_HANDLE *usb_device, uint8_t address)
{
    USB_ENDPOINT *ep;

    /* If this is an IN endpoint. */
    if ((0x80 & address) == 0x80)
    {
        /* Get the required endpoint. */
        ep = &usb_device->device.in_ep[address & 0x7F];
    }
    else
    {
        /* This is an OUT endpoint. */
        ep = &usb_device->device.out_ep[address];
    }

    /* Update endpoint. */
    ep->is_stall = 1;
    ep->num = address & 0x7F;
    ep->is_in = ((address & 0x80) == 0x80);

    /* Stall this endpoint.  */
    usb_function_stm32f407_ep_set_stall(usb_device, ep);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_stall */

/*
 * usb_fun_endpoint_clear_stall
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * This will clear stall on an endpoint.
 */
uint32_t usb_fun_endpoint_clear_stall(USB_STM32F407_HANDLE *usb_device, uint8_t address)
{
    USB_ENDPOINT *ep;

    /* If this is an IN endpoint. */
    if ((0x80 & address) == 0x80)
    {
        /* Get the required endpoint. */
        ep = &usb_device->device.in_ep[address & 0x7F];
    }
    else
    {
        /* This is an OUT endpoint. */
        ep = &usb_device->device.out_ep[address];
    }

    /* Update endpoint. */
    ep->is_stall = 0;
    ep->num = address & 0x7F;
    ep->is_in = ((address & 0x80) == 0x80);

    /* Clear stall for this endpoint.  */
    usb_function_stm32f407_ep_clear_stall(usb_device, ep);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_clear_stall */

/*
 * usb_fun_endpoint_flush
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * This will flush data on an endpoint.
 */
uint32_t usb_fun_endpoint_flush(USB_STM32F407_HANDLE *usb_device, uint8_t address)
{
    /* If this is an IN endpoint. */
    if ((address & 0x80) == 0x80)
    {
        /* Flush TX FIFO. */
        usb_stm32f407_flush_tx_fifo(usb_device, address & 0x7F);
    }
    else
    {
        /* Flush RX FIFO. */
        usb_stm32f407_flush_rx_fifo(usb_device);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_endpoint_flush */

/*
 * usb_fun_endpoint_set_address
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * This will set address for an endpoint.
 */
void usb_fun_endpoint_set_address(USB_STM32F407_HANDLE *usb_device, uint8_t address)
{
    USB_STM32F407_DCFG dcfg;

    /* Initialize configuration to be set. */
    dcfg.d32 = 0;
    dcfg.b.devaddr = (uint32_t)(address & MASK_N_BITS(7));

    /* Update configuration. */
    OS_MASK_REG32(&usb_device->regs.DREGS->DCFG, 0, dcfg.d32);

} /* usb_fun_endpoint_set_address */

/*
 * usb_fun_endpoint_get_status
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * This will return the an endpoint's status.
 */
uint32_t usb_fun_endpoint_get_status(USB_STM32F407_HANDLE *usb_device, uint8_t address)
{
    USB_ENDPOINT *ep;
    uint32_t status = 0;

    /* If this is an IN endpoint. */
    if ((0x80 & address) == 0x80)
    {
        /* Get the required endpoint. */
        ep = &usb_device->device.in_ep[address & 0x7F];
    }
    else
    {
        /* This is an OUT endpoint. */
        ep = &usb_device->device.out_ep[address];
    }

    /* Get endpoint status. */
    status = usb_function_stm32f407_ep_get_status(usb_device ,ep);

    /* Return the current status */
    return (status);

} /* usb_fun_endpoint_get_status */

/*
 * usb_fun_endpoint_set_status
 * @usb_device: USB device instance.
 * @address: Endpoint address.
 * This will set an endpoint's status.
 */
void usb_fun_endpoint_set_status(USB_STM32F407_HANDLE *usb_device, uint8_t address, uint32_t status)
{
    USB_ENDPOINT *ep;

    /* If this is an IN endpoint. */
    if ((0x80 & address) == 0x80)
    {
        /* Get the required endpoint. */
        ep = &usb_device->device.in_ep[address & 0x7F];
    }
    else
    {
        /* This is an OUT endpoint. */
        ep = &usb_device->device.out_ep[address];
    }

    /* Set endpoint status. */
    usb_function_stm32f407_ep_set_status(usb_device, ep, status);

} /* usb_fun_endpoint_set_status */

#endif /* USB_FUNCTION */
