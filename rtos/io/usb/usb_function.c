/*
 * usb_function.c
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

/* Function prototypes. */
static uint32_t usb_fun_setup_stage(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_data_out_stage(USB_STM32F407_HANDLE *, uint8_t);
static uint32_t usb_fun_data_in_stage(USB_STM32F407_HANDLE *, uint8_t);
static uint32_t usb_fun_reset(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_resume(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_suspend(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_sof(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_set_cfg(USB_STM32F407_HANDLE *, uint8_t);
static uint32_t usb_fun_clear_cfg(USB_STM32F407_HANDLE *, uint8_t);
static uint32_t usb_fun_iso_in_incomplete(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_iso_out_incomplete(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_dev_connected(USB_STM32F407_HANDLE *);
static uint32_t usb_fun_dev_disconnected(USB_STM32F407_HANDLE *);

/* USB interrupt callbacks for USB function device. */
static USB_FUN_INTERRUPT_CB interrupt_cb =
{
    .setup_stage = &usb_fun_setup_stage,
    .data_out_stage = &usb_fun_data_out_stage,
    .data_in_stage = &usb_fun_data_in_stage,
    .reset = &usb_fun_reset,
    .resume = &usb_fun_resume,
    .suspend = &usb_fun_suspend,
    .sof = &usb_fun_sof,
    .iso_in_incomplete = &usb_fun_iso_in_incomplete,
    .iso_out_incomplete = &usb_fun_iso_out_incomplete,
    .connected = &usb_fun_dev_connected,
    .disconnected = &usb_fun_dev_disconnected,
};
USB_FUN_INTERRUPT_CB *usb_function_interrupt_cb = &interrupt_cb;

static USB_FUN_REQ_CB request_cb =
{
    .set_cfg = &usb_fun_set_cfg,
    .clear_cfg = &usb_fun_clear_cfg,
};
USB_FUN_REQ_CB *usb_function_requset_cb = &request_cb;

/*
 * usb_function_init
 * @usb_device: USB device instance to be initialized.
 * @class_cb: USB function callbacks.
 * This will initialize a new USB function device.
 */
void usb_function_init(USB_STM32F407_HANDLE *usb_device, USB_FUN_CB *class_cb)
{
    /* Register class. */
    usb_device->device.class_cb = class_cb;

    /* Initialize core parameters. */
    usb_fun_device_init(usb_device);

} /* usb_function_init */

/*
 * usb_fun_setup_stage
 * @usb_device: USB device instance.
 * This will handle setup stage.
 */
static uint32_t usb_fun_setup_stage(USB_STM32F407_HANDLE *usb_device)
{
    USB_SETUP_REQ req;

    /* Parse this setup request. */
    usb_fun_parse_setup_request(usb_device, &req);

    /* Process this request. */
    switch (req.bmRequest & 0x1F)
    {
    case (USB_REQ_RECIPIENT_DEVICE):
        usb_fun_std_dev_request(usb_device, &req);
        break;

    case (USB_REQ_RECIPIENT_INTERFACE):
        usb_fun_std_interface_request(usb_device, &req);
        break;

    case (USB_REQ_RECIPIENT_ENDPOINT):
        usb_fun_std_endpoint_request(usb_device, &req);
        break;

    default:
        /* Stall this endpoint. */
        usb_fun_endpoint_stall(usb_device, (req.bmRequest & 0x80));
        break;
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_setup_stage */

/*
 * usb_fun_data_out_stage
 * @usb_device: USB device instance.
 * @epnum: Endpoint number for which we need to receive data.
 * This will handle data out stage for an endpoint.
 */
static uint32_t usb_fun_data_out_stage(USB_STM32F407_HANDLE *usb_device, uint8_t epnum)
{
    USB_ENDPOINT *ep;

    /* Check if this was a control endpoint. */
    if (epnum == 0)
    {
        /* Get the control endpoint. */
        ep = &usb_device->device.out_ep[0];

        /* Check if some data is needed to be received. */
        if (usb_device->device.state == USB_STM32F407_EP0_DATA_OUT)
        {
            /* Check if we need to receive in parts. */
            if (ep->rem_data_len > ep->maxpacket)
            {
                /* Update remaining data length. */
                ep->rem_data_len -=  ep->maxpacket;

                /* Check if we are using DMA. */
                if (usb_device->cfg.dma_enable == 1)
                {
                    /* Update the buffer pointer. */
                    ep->xfer_buff += ep->maxpacket;
                }

                /* Receive a part from device. */
                usb_fun_control_continue_rx(usb_device, ep->xfer_buff, MIN(ep->rem_data_len, ep->maxpacket));
            }
            else
            {
                /* Check if we need to receive some control data in class. */
                if ((usb_device->device.class_cb->ep0_rx_ready) && (usb_device->device.status == USB_FUN_STATE_CONFIGURED))
                {
                    /* Receive control data. */
                    usb_device->device.class_cb->ep0_rx_ready(usb_device);
                }

                /* Send control status. */
                usb_fun_control_send_status(usb_device);
            }
        }
    }

    /* This is not a control endpoint. */
    else if ((usb_device->device.class_cb->data_out) && (usb_device->device.status == USB_FUN_STATE_CONFIGURED))
    {
        /* Check if we need to send some data form the class driver. */
        usb_device->device.class_cb->data_out(usb_device, epnum);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_data_out_stage */

/*
 * usb_fun_data_in_stage
 * @usb_device: USB device instance.
 * @epnum: Endpoint number for which we need to send data.
 * This will handle data in stage for an endpoint.
 */
static uint32_t usb_fun_data_in_stage(USB_STM32F407_HANDLE *usb_device, uint8_t epnum)
{
    USB_ENDPOINT *ep;

    /* If this is control endpoint. */
    if (epnum == 0)
    {
        /* Get the pointer to control endpoint. */
        ep = &usb_device->device.in_ep[0];

        /* If some data is needed to be sent. */
        if (usb_device->device.state == USB_STM32F407_EP0_DATA_IN)
        {
            /* If we need to send in parts. */
            if (ep->rem_data_len > ep->maxpacket)
            {
                /* Update the remaining length. */
                ep->rem_data_len -=  ep->maxpacket;

                /* If we are using DMA. */
                if (usb_device->cfg.dma_enable == 1)
                {
                    /* Update receive buffer pointer. */
                    ep->xfer_buff += ep->maxpacket;
                }

                /* Send this part. */
                usb_fun_control_continue_tx(usb_device, ep->xfer_buff, ep->rem_data_len);
            }

            else
            {
                /* Check if we need send a zero length packet. */
                if (((ep->total_data_len % ep->maxpacket) == 0) && (ep->total_data_len >= ep->maxpacket) && (ep->total_data_len < ep->ctl_data_len ))
                {
                    usb_fun_control_continue_tx(usb_device, NULL, 0);
                    ep->ctl_data_len = 0;
                }

                else
                {
                    /* Check if we need to handle this in class driver. */
                    if ((usb_device->device.class_cb->ep0_tx_sent != NULL) && (usb_device->device.status == USB_FUN_STATE_CONFIGURED))
                    {
                        usb_device->device.class_cb->ep0_tx_sent(usb_device);
                    }

                    usb_fun_control_receive_status(usb_device);
                }
            }
        }
    }

    /* This is not a control endpoint. */
    else if ((usb_device->device.class_cb->data_in) && (usb_device->device.status == USB_FUN_STATE_CONFIGURED))
    {
        /* Send data is class driver. */
        usb_device->device.class_cb->data_in(usb_device, epnum);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_data_in_stage */

/*
 * usb_fun_reset
 * @usb_device: USB device instance.
 * Handles reset event.
 */
static uint32_t usb_fun_reset(USB_STM32F407_HANDLE *usb_device)
{
    /* Open EP0 OUT. */
    usb_fun_endpoint_open(usb_device, 0x00, USB_STM32F407_MAX_EP0_SIZE, EP_TYPE_CTRL);

    /* Open EP0 IN. */
    usb_fun_endpoint_open(usb_device, 0x80, USB_STM32F407_MAX_EP0_SIZE, EP_TYPE_CTRL);

    /* Update device status. */
    usb_device->device.status = USB_FUN_STATE_DEFAULT;

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_reset */

/*
 * usb_fun_resume
 * @usb_device: USB device instance.
 * Handles resume event.
 */
static uint32_t usb_fun_resume(USB_STM32F407_HANDLE *usb_device)
{
    /* Update device status. */
    usb_device->device.status = USB_FUN_STATE_CONFIGURED;

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_resume */

/*
 * usb_fun_suspend
 * @usb_device: USB device instance.
 * Handles suspend event.
 */
static uint32_t usb_fun_suspend(USB_STM32F407_HANDLE *usb_device)
{
    /* Update device status. */
    usb_device->device.status = USB_FUN_STATE_SUSPENDED;

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_suspend */

/*
 * usb_fun_sof
 * @usb_device: USB device instance.
 * Handles start of frame event.
 */
static uint32_t usb_fun_sof(USB_STM32F407_HANDLE *usb_device)
{
    if (usb_device->device.class_cb->sof)
    {
        usb_device->device.class_cb->sof(usb_device);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_sof */

/*
 * usb_fun_set_cfg
 * @usb_device: USB device instance.
 * @cfgidx: Configuration index.
 * Handles clear configuration event.
 */
static uint32_t usb_fun_set_cfg(USB_STM32F407_HANDLE *usb_device, uint8_t cfgidx)
{
    /* Initialize class driver. */
    usb_device->device.class_cb->init(usb_device, cfgidx);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_set_cfg */

/*
 * usb_fun_clear_cfg
 * @usb_device: USB device instance.
 * @cfgidx: Configuration index.
 * Handles clear configuration event.
 */
static uint32_t usb_fun_clear_cfg(USB_STM32F407_HANDLE *usb_device, uint8_t cfgidx)
{
    usb_device->device.class_cb->deinit(usb_device, cfgidx);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_clear_cfg */

/*
 * usb_fun_iso_in_incomplete
 * @usb_device: USB device instance.
 * Handles ISO in incomplete event.
 */
static uint32_t usb_fun_iso_in_incomplete(USB_STM32F407_HANDLE *usb_device)
{
    if (usb_device->device.class_cb->iso_in_incomplete)
    {
        usb_device->device.class_cb->iso_in_incomplete(usb_device);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_iso_in_incomplete */

/*
 * usb_fun_iso_out_incomplete
 * @usb_device: USB device instance.
 * Handles ISO out incomplete event.
 */
static uint32_t usb_fun_iso_out_incomplete(USB_STM32F407_HANDLE *usb_device)
{
    if (usb_device->device.class_cb->iso_out_incomplete)
    {
        usb_device->device.class_cb->iso_out_incomplete(usb_device);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_iso_out_incomplete */

/*
 * usb_fun_dev_connected
 * @usb_device: USB device instance.
 * This function is called when USB device is connected, detected through
 * VBUS sensing.
 */
static uint32_t usb_fun_dev_connected(USB_STM32F407_HANDLE *usb_device)
{
    /* Call class callback. */
    if (usb_device->device.class_cb->connected)
    {
        usb_device->device.class_cb->connected(usb_device);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_dev_connected */

/*
 * usb_fun_dev_disconnected
 * @usb_device: USB device instance.
 * This function is called when USB device is disconnected, detected through
 * VBUS sensing.
 */
static uint32_t usb_fun_dev_disconnected(USB_STM32F407_HANDLE *usb_device)
{
    /* Call class callback. */
    if (usb_device->device.class_cb->disconnected)
    {
        usb_device->device.class_cb->disconnected(usb_device);
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_dev_disconnected */

#endif /* USB_FUNCTION */
