/*
 * usb_fun_request.c
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

/* Function prototypes. */
static void usb_fun_get_descriptor(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
static void usb_fun_set_address(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
static void usb_fun_set_config(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
static void usb_fun_get_config(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
static void usb_fun_get_status(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
static void usb_fun_set_feature(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);
static void usb_fun_clear_feature(USB_STM32F407_HANDLE *, USB_SETUP_REQ *);

/*
 * usb_fun_std_dev_request
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handle standard device request.
 */
uint32_t usb_fun_std_dev_request(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ  *req)
{
    /* Process this request accordingly. */
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
        usb_fun_get_descriptor (usb_device, req) ;
        break;

    case USB_REQ_SET_ADDRESS:
        usb_fun_set_address(usb_device, req);
        break;

    case USB_REQ_SET_CONFIGURATION:
        usb_fun_set_config (usb_device, req);
        break;

    case USB_REQ_GET_CONFIGURATION:
        usb_fun_get_config (usb_device, req);
        break;

    case USB_REQ_GET_STATUS:
        usb_fun_get_status (usb_device, req);
        break;

    case USB_REQ_SET_FEATURE:
        usb_fun_set_feature (usb_device, req);
        break;

    case USB_REQ_CLEAR_FEATURE:
        usb_fun_clear_feature (usb_device, req);
        break;

    default:
        /* Should not happen return an error. */
        usb_fun_control_error(usb_device, req);
        break;
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_std_dev_request */

/*
 * usb_fun_std_interface_request
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handle standard interface requests.
 */
uint32_t usb_fun_std_interface_request(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    /* Process this request according to device status. */
    switch (usb_device->device.status)
    {
    /* If we are configured. */
    case USB_FUN_STATE_CONFIGURED:

        /* Verify the endpoint number. */
        if (LOBYTE(req->wIndex) <= USB_FUN_ITF_MAX_NUM)
        {
            /* Process this request. */
            usb_device->device.class_cb->setup(usb_device, req);

            /* Unable to process this request. */
            if (req->wLength == 0)
            {
                /* Send control status. */
                usb_fun_control_send_status(usb_device);
            }
        }

        else
        {
            /* Send a control error. */
            usb_fun_control_error(usb_device, req);
        }

        break;

    default:
        /* Send a control error. */
        usb_fun_control_error(usb_device, req);

        break;
    }

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_std_interface_request */

/*
 * usb_fun_std_endpoint_request
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handle standard endpoint requests.
 */
uint32_t usb_fun_std_endpoint_request(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    /* This needs to be aligned. */
    uint32_t ep_status;
    uint8_t ep_addr;

    /* Get the endpoint for which we need to process this request. */
    ep_addr  = LOBYTE(req->wIndex);

    /* Handle this request. */
    switch (req->bRequest)
    {

    /* Set feature request. */
    case (USB_REQ_SET_FEATURE):

        /* Handle this request according to device status. */
        switch (usb_device->device.status)
        {
        /* Device is being addressed. */
        case (USB_FUN_STATE_ADDRESSED):

            /* If this not IN or OUT control endpoint. */
            if ((ep_addr != 0x00) && (ep_addr != 0x80))
            {
                /* Stall this endpoint. */
                usb_fun_endpoint_stall(usb_device, ep_addr);
            }

            break;

        /* Device is configured. */
        case (USB_FUN_STATE_CONFIGURED):

            /* If we need to halt the feature. */
            if (req->wValue == USB_FEATURE_EP_HALT)
            {
                /* If this not IN or OUT control endpoint. */
                if ((ep_addr != 0x00) && (ep_addr != 0x80))
                {
                    /* Stall this endpoint. */
                    usb_fun_endpoint_stall(usb_device, ep_addr);
                }
            }

            /* Setup USB class. */
            usb_device->device.class_cb->setup(usb_device, req);

            /* Send status. */
            usb_fun_control_send_status(usb_device);

            break;

        default:
            /* Send an error status. */
            usb_fun_control_error(usb_device, req);

            break;
        }
        break;

    case (USB_REQ_CLEAR_FEATURE):

        /* Handle this request according to device status. */
        switch (usb_device->device.status)
        {
        /* If device is being addressed. */
        case (USB_FUN_STATE_ADDRESSED):

            /* If this not IN or OUT control endpoint. */
            if ((ep_addr != 0x00) && (ep_addr != 0x80))
            {
                /* Stall this endpoint. */
                usb_fun_endpoint_stall(usb_device, ep_addr);
            }

            break;

        /* Device is configured. */
        case (USB_FUN_STATE_CONFIGURED):

            /* If we need to halt the feature. */
            if (req->wValue == USB_FEATURE_EP_HALT)
            {
                /* If this not IN or OUT control endpoint. */
                if ((ep_addr != 0x00) && (ep_addr != 0x80))
                {
                    /* Clear stall for this endpoint. */
                    usb_fun_endpoint_clear_stall(usb_device, ep_addr);

                    /* Setup USB class. */
                    usb_device->device.class_cb->setup(usb_device, req);
                }

                /* Send status. */
                usb_fun_control_send_status(usb_device);
            }
            break;

        default:
            /* Send an error status. */
            usb_fun_control_error(usb_device, req);

            break;
        }

        break;

    /* If status is requested. */
    case (USB_REQ_GET_STATUS):

        /* Handle this request according to device status. */
        switch (usb_device->device.status)
        {
        /* If device is being addressed. */
        case (USB_FUN_STATE_ADDRESSED):

            /* If this not IN or OUT control endpoint. */
            if ((ep_addr != 0x00) && (ep_addr != 0x80))
            {
                /* Stall this endpoint. */
                usb_fun_endpoint_stall(usb_device, ep_addr);
            }

            break;

        /* Device is configured. */
        case (USB_FUN_STATE_CONFIGURED):

            /* If this is IN endpoint. */
            if (ep_addr & 0x80)
            {
                /* Check if we are stalled. */
                if(usb_device->device.in_ep[ep_addr & 0x7F].is_stall)
                {
                    ep_status = 0x0001;
                }

                else
                {
                    ep_status = 0x0000;
                }
            }

            /* If this is OUT endpoint. */
            else if (!(ep_addr & 0x80))
            {
                /* Check if we are stalled. */
                if(usb_device->device.out_ep[ep_addr].is_stall)
                {
                    ep_status = 0x0001;
                }

                else
                {
                    ep_status = 0x0000;
                }
            }

            /* Send the endpoint status. */
            usb_fun_control_tx (usb_device, (uint8_t *)&ep_status, 2);

            break;

        default:
            /* Send an error status. */
            usb_fun_control_error(usb_device, req);

            break;
        }

        break;

    default:
        break;
    }

    /* Always return status. */
    return (SUCCESS);

} /* usb_fun_std_endpoint_request */

/*
 * usb_fun_get_descriptor
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handle get descriptor requests.
 */
static void usb_fun_get_descriptor(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    uint16_t len = 0;
    uint8_t *pbuf;

    switch (req->wValue >> 8)
    {
    /* If we need device descriptor. */
    case (USB_DESC_TYPE_DEVICE):

        /* Populate the device descriptor. */
        pbuf = usb_device->device.class_cb->desc_cb->get_device_desc(usb_device->cfg.speed, &len);

        if ((req->wLength == 64) ||( usb_device->device.status == USB_FUN_STATE_DEFAULT))
        {
            len = 8;
        }

        break;

    /* If we need configuration descriptor. */
    case (USB_DESC_TYPE_CONFIG):

        /* Populate the configuration descriptor. */
        pbuf = (uint8_t *)usb_device->device.class_cb->get_config_descriptor(usb_device->cfg.speed, &len);

        /* Push the remaining configuration descriptors for HS mode. */
        if (usb_device->cfg.speed == USB_STM32F407_SPEED_FULL)
        {
            pbuf = (uint8_t *)usb_device->device.class_cb->get_other_config_descriptor(usb_device->cfg.speed, &len);
        }

        /* Update the descriptor type. */
        pbuf[1] = USB_DESC_TYPE_CONFIG;

        /* Update the descriptor pointer. */
        usb_device->device.config_desc = pbuf;

        break;

    /* If we need a string descriptor. */
    case (USB_DESC_TYPE_STRING):

        /* Save the number of bytes that can be used to convert the string. */
        len = USB_STRING_DESC_SIZE;

        /* Return required descriptor. */
        switch ((uint8_t)(req->wValue))
        {
        case (USB_DEV_IDX_LANGID_STR):
            pbuf = usb_device->device.class_cb->desc_cb->get_lang_id_desc(usb_device->cfg.speed, &len);
            break;

        case (USB_DEV_IDX_MFC_STR):
            pbuf = usb_device->device.class_cb->desc_cb->get_mfg_str_desc(usb_device->cfg.speed, usb_device->device.str_desc_buffer, &len);
            break;

        case (USB_DEV_IDX_PRODUCT_STR):
            pbuf = usb_device->device.class_cb->desc_cb->get_product_str_desc(usb_device->cfg.speed, usb_device->device.str_desc_buffer, &len);
            break;

        case (USB_DEV_IDX_SERIAL_STR):
            pbuf = usb_device->device.class_cb->desc_cb->get_serial_number_str_desc(usb_device->cfg.speed, usb_device->device.str_desc_buffer, &len);
            break;

        case (USB_DEV_IDX_CONFIG_STR):
            pbuf = usb_device->device.class_cb->desc_cb->get_cfg_str_desc(usb_device->cfg.speed, usb_device->device.str_desc_buffer, &len);
            break;

        case (USB_DEV_IDX_INTERFACE_STR):
            pbuf = usb_device->device.class_cb->desc_cb->get_iface_str_desc(usb_device->cfg.speed, usb_device->device.str_desc_buffer, &len);
            break;

        default:
#ifdef USB_SUPPORT_STRING_DESC
            if (usb_device->device.class_cb->get_usr_str_descriptor)
            {
                pbuf = usb_device->device.class_cb->get_usr_str_descriptor(usb_device->cfg.speed, req->wValue, &len);
            }
            else
            {
#endif
                /* Not supported return an error. */
                usb_fun_control_error(usb_device, req);
#ifdef USB_SUPPORT_STRING_DESC
            }
#endif
            break;
        }

        break;

    case (USB_DESC_TYPE_DEVICE_QUALIFIER):
        /* If we are using HS. */
        if (usb_device->cfg.speed == USB_STM32F407_SPEED_HIGH)
        {
            /* Return configuration descriptor. */
            pbuf = (uint8_t *)usb_device->device.class_cb->get_config_descriptor(usb_device->cfg.speed, &len);
        }
        else
        {
            /* Should not happen return an error. */
            usb_fun_control_error(usb_device, req);
        }

        break;

    case (USB_DESC_TYPE_SPEED_CONFIG):
        /* If we are using HS. */
        if (usb_device->cfg.speed == USB_STM32F407_SPEED_HIGH)
        {
            /* Return remaining configuration descriptor. */
            pbuf = (uint8_t *)usb_device->device.class_cb->get_other_config_descriptor(usb_device->cfg.speed, &len);
            pbuf[1] = USB_DESC_TYPE_SPEED_CONFIG;
        }
        else
        {
            /* Should not happen return an error. */
            usb_fun_control_error(usb_device, req);
        }

        break;

    default:
        /* Should not happen return an error. */
        usb_fun_control_error(usb_device, req);

        return;
    }

    /* Check if we need to send some data. */
    if((len != 0) && (req->wLength != 0))
    {
        /* Calculate the length of data we need to send. */
        len = MIN(len, req->wLength);

        /* Send data on control on this endpoint. */
        usb_fun_control_tx(usb_device, pbuf, len);
    }

} /* usb_fun_get_descriptor */

/*
 * usb_fun_set_address
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Set device address.
 */
static void usb_fun_set_address(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    uint8_t  dev_addr;

    /* Check if this is IN request. */
    if ((req->wIndex == 0) && (req->wLength == 0))
    {
        dev_addr = (uint8_t)(req->wValue) & 0x7F;

        /* If device is configured. */
        if (usb_device->device.status == USB_FUN_STATE_CONFIGURED)
        {
            usb_fun_control_error(usb_device, req);
        }
        else
        {
            /* Set device request. */
            usb_device->device.address = dev_addr;

            /* Set endpoint address. */
            usb_fun_endpoint_set_address(usb_device, dev_addr);

            /* Send status. */
            usb_fun_control_send_status(usb_device);

            /* If we have an address, update device status. */
            if (dev_addr != 0)
            {
                usb_device->device.status = USB_FUN_STATE_ADDRESSED;
            }
            else
            {
                usb_device->device.status = USB_FUN_STATE_DEFAULT;
            }
        }
    }

    else
    {
        /* Should not happen return an error. */
        usb_fun_control_error(usb_device, req);
    }

} /* usb_fun_set_address */

/*
 * usb_fun_set_config
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Set device configuration.
 */
static void usb_fun_set_config(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    uint8_t cfgidx;

    /* Get the configuration index. */
    cfgidx = (uint8_t)(req->wValue);

    /* If this is a valid index. */
    if (cfgidx > USB_FUN_CFG_MAX_NUM)
    {
        /* Should not happen return an error. */
        usb_fun_control_error(usb_device, req);
    }
    else
    {
        /* Process request according to device status. */
        switch (usb_device->device.status)
        {
        /* If we are being addressed. */
        case (USB_FUN_STATE_ADDRESSED):

            /* If we have a configuration index. */
            if (cfgidx)
            {
                /* Populate configuration registration. */
                usb_device->device.config = cfgidx;
                usb_device->device.status = USB_FUN_STATE_CONFIGURED;
                usb_function_requset_cb->set_cfg(usb_device, cfgidx);
                usb_fun_control_send_status(usb_device);
            }
            else
            {
                /* Send control status. */
                usb_fun_control_send_status(usb_device);
            }
            break;

        /* We are configured. */
        case (USB_FUN_STATE_CONFIGURED):

            /* If this is default index. */
            if (cfgidx == 0)
            {
                usb_device->device.status = USB_FUN_STATE_ADDRESSED;
                usb_device->device.config = cfgidx;
                usb_function_requset_cb->clear_cfg(usb_device, cfgidx);
                usb_fun_control_send_status(usb_device);
            }

            /* If we need to process new configuration. */
            else if (cfgidx != usb_device->device.config)
            {
                /* Clear old configuration. */
                usb_function_requset_cb->clear_cfg(usb_device, usb_device->device.config);

                /* Set new configuration. */
                usb_device->device.config = cfgidx;
                usb_function_requset_cb->set_cfg(usb_device, cfgidx);
                usb_fun_control_send_status(usb_device);
            }

            else
            {
                /* Send control status. */
                usb_fun_control_send_status(usb_device);
            }

            break;

        default:
            /* Should not happen return an error. */
            usb_fun_control_error(usb_device, req);

            break;
        }
    }
} /* usb_fun_set_config */

/*
 * usb_fun_get_config
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Set device configuration.
 */
static void usb_fun_get_config(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    /* This needs to be aligned. */
    uint32_t default_cfg = 0;

    if (req->wLength != 1)
    {
        /* Should not happen return an error. */
        usb_fun_control_error(usb_device, req);
    }
    else
    {
        /* Process request according to device status. */
        switch (usb_device->device.status)
        {
        /* if we are being addressed. */
        case (USB_FUN_STATE_ADDRESSED):

            /* Send default configuration. */
            usb_fun_control_tx (usb_device, (uint8_t *)&default_cfg, 1);

            break;

        /* If we are configured. */
        case (USB_FUN_STATE_CONFIGURED):

            /* Send device configuration. */
            usb_fun_control_tx (usb_device, &usb_device->device.config, 1);

            break;

        default:

            /* Should not happen send error. */
            usb_fun_control_error(usb_device, req);

            break;
        }
    }
} /* usb_fun_get_config */

/*
 * usb_fun_get_status
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handles get device status.
 */
static void usb_fun_get_status(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    /* This needs to be aligned. */
    uint32_t cfg_status = 0;

    switch (usb_device->device.status)
    {

    /* If we are addressed or configured. */
    case (USB_FUN_STATE_ADDRESSED):
    case (USB_FUN_STATE_CONFIGURED):

        /* If we support remote wake up. */
        if (usb_device->device.remote_wakeup)
        {
            /* Populate status. */
            cfg_status = (USB_CONFIG_SELF_POWERED | USB_CONFIG_REMOTE_WAKEUP);
        }
        else
        {
            /* Populate status. */
            cfg_status = (USB_CONFIG_SELF_POWERED);
        }

        /* Send device status. */
        usb_fun_control_tx (usb_device, (uint8_t *)&cfg_status, 1);

        break;

    default:
        usb_fun_control_error(usb_device, req);
        break;
    }

} /* usb_fun_get_status */

/*
 * usb_fun_set_feature
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handles set device feature request.
 */
static void usb_fun_set_feature(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    USB_STM32F407_DCTL dctl;
    uint8_t test_mode = 0;

    /* If we need to process remote wake-up. */
    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
    {
        /* Process this request. */
        usb_device->device.remote_wakeup = 1;
        usb_device->device.class_cb->setup(usb_device, req);

        /* Send status. */
        usb_fun_control_send_status(usb_device);
    }

    else if ((req->wValue == USB_FEATURE_TEST_MODE) && ((req->wIndex & 0xFF) == 0))
    {
        dctl.d32 = OS_READ_REG32(&usb_device->regs.DREGS->DCTL);

        /* Get required test. */
        test_mode = (uint8_t)(req->wIndex >> 8);

        /* Process the required test. */
        switch (test_mode)
        {
        /* Test J. */
        case (1):
            dctl.b.tstctl = 1;
            break;

        /* Test K. */
        case (2):
            dctl.b.tstctl = 2;
            break;

        /* Test SE0_NAK. */
        case (3):
            dctl.b.tstctl = 3;
            break;

        /* Test packet. */
        case (4):
            dctl.b.tstctl = 4;
            break;

        /* Test force enable. */
        case (5):
            dctl.b.tstctl = 5;
            break;
        }

        /* Update control register. */
        OS_WRITE_REG32(&usb_device->regs.DREGS->DCTL, dctl.d32);

        /* Send status. */
        usb_fun_control_send_status(usb_device);
    }

} /* usb_fun_set_feature */

/*
 * usb_fun_clear_feature
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handles set device feature request.
 */
static void usb_fun_clear_feature(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    /* Process this request according to device status. */
    switch (usb_device->device.status)
    {
    /* If we are addressed or configured. */
    case (USB_FUN_STATE_ADDRESSED):
    case (USB_FUN_STATE_CONFIGURED):

        /* If we need to handle remote wake up. */
        if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
        {
            /* Process this request. */
            usb_device->device.remote_wakeup = 0;
            usb_device->device.class_cb->setup(usb_device, req);

            /* Send status. */
            usb_fun_control_send_status(usb_device);
        }

        break;

    default:

        /* Should not happen return error. */
        usb_fun_control_error(usb_device, req);

        break;
    }
} /* usb_fun_clear_feature */

/*
 * usb_fun_parse_setup_request
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Parse incoming request.
 */
void usb_fun_parse_setup_request(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    req->bmRequest    = *(uint8_t *)(usb_device->device.setup_packet);
    req->bRequest     = *(uint8_t *)(usb_device->device.setup_packet + 1);
    req->wValue       = SWAPBYTE(usb_device->device.setup_packet + 2);
    req->wIndex       = SWAPBYTE(usb_device->device.setup_packet + 4);
    req->wLength      = SWAPBYTE(usb_device->device.setup_packet + 6);

    usb_device->device.in_ep[0].ctl_data_len = req->wLength;
    usb_device->device.state = USB_STM32F407_EP0_SETUP;

} /* usb_fun_parse_setup_request */

/*
 * usb_fun_control_error
 * @usb_device: USB device instance.
 * @req: Request needed to be processed.
 * Handle control error.
 */
void usb_fun_control_error(USB_STM32F407_HANDLE *usb_device, USB_SETUP_REQ *req)
{
    /* If we are processing IN request. */
    if (req->bmRequest & 0x80)
    {
        /* Stall IN control endpoint. */
        usb_fun_endpoint_stall(usb_device, 0x80);
    }

    else
    {
        if(req->wLength == 0)
        {
            /* Stall IN control endpoint. */
            usb_fun_endpoint_stall(usb_device, 0x80);
        }

        else
        {
            /* Stall OUT control endpoint. */
            usb_fun_endpoint_stall(usb_device, 0);
        }
    }

    /* Send control endpoint. */
    usb_function_stm32f407_ep0_start_out(usb_device);

} /* usb_fun_control_error */

/*
 * usb_fun_get_string
 * @desc: String descriptor needed to be converted.
 * @unicode: Buffer where string will be returned.
 * @len: On input will contain number of bytes that can be copied in the
 *  buffer, on output will contain number of bytes actually converted.
 * Convert ASCII string into UNICODE encoded string.
 */
void usb_fun_get_string(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
    uint16_t length;
    uint8_t idx = 0;

    /* If we have a descriptor. */
    if (desc != NULL)
    {
        /* Calculate required length. */
        length = (uint16_t)((strlen((char *)desc) * 2) + 2);

        /* Check if we can convert this string in the provided. */
        if (*len >= length)
        {
            /* Put length on the start of string. */
            unicode[idx++] = (uint8_t)*len;

            /* Put descriptor type. */
            unicode[idx++] = USB_DESC_TYPE_STRING;

            /* Convert and save the descriptor. */
            while (*desc != '\0')
            {
                /* Convert the descriptor. */
                unicode[idx++] = *desc++;
                unicode[idx++] = 0x00;
            }

            /* Return number of bytes converted. */
            *len = length;
        }
        else
        {
            /* Nothing was converted. */
            *len = 0;

            /* Should not happen. */
            OS_ASSERT(TRUE);
        }
    }

} /* usb_fun_get_string */

#endif /* USB_FUNCTION */
