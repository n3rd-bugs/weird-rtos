/*
 * usb_fun_cdc.c
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

/* Internal function prototypes. */
static uint32_t usb_fun_cdc_init(void *, uint8_t);
static uint32_t usb_fun_cdc_deinit(void *, uint8_t);
static uint32_t usb_fun_cdc_setup (void *, USB_SETUP_REQ *);
static uint32_t usb_fun_cdc_ep0_rx_ready(void *);
static uint32_t usb_fun_cdc_data_in(void *, uint8_t);
static uint32_t usb_fun_cdc_data_out(void *, uint8_t);
static uint32_t usb_fun_cdc_sof(void *);
static uint8_t *usb_fun_cdc_get_cfg_desc(uint8_t, uint16_t *);
static uint8_t *usb_fun_cdc_get_other_cfg_desc(uint8_t, uint16_t *);
static uint8_t *usb_fun_cdc_device_descriptor(uint8_t, uint16_t *);
static uint8_t *usb_fun_cdc_langid_descriptor(uint8_t, uint16_t *);
static uint8_t *usb_fun_cdc_manufacturer_descriptor(uint8_t, uint8_t *, uint16_t *);
static uint8_t *usb_fun_cdc_product_descriptor(uint8_t, uint8_t *, uint16_t *);
static uint8_t *usb_fun_cdc_serial_number_descriptor(uint8_t, uint8_t *, uint16_t *);
static uint8_t *usb_fun_cdc_config_descriptor(uint8_t, uint8_t *, uint16_t *);
static uint8_t *usb_fun_cdc_iface_descriptor(uint8_t, uint8_t *, uint16_t *);

/* USB standard device descriptor. */
static uint8_t usb_fun_cdc_device_desc[USB_DEVICE_DESC_SIZE] __attribute__ ((aligned (4))) =
{
    0x12,                       /* Descriptor length. */
    USB_DEVICE_DESC_TYPE,       /* Descriptor type. */
    0x00,                       /* USB. */
    0x02,
    0x00,                       /* Device class. */
    0x00,                       /* Device sub class. */
    0x00,                       /* Device protocol. */
    USB_STM32F407_MAX_EP0_SIZE, /* Max packet size. */
    LOBYTE(USB_FUN_CDC_VID),    /* Vendor ID. */
    HIBYTE(USB_FUN_CDC_VID),    /* Vendor ID. */
    LOBYTE(USB_FUN_CDC_PID),    /* Product ID. */
    HIBYTE(USB_FUN_CDC_PID),    /* Product ID. */
    0x00,
    0x02,
    USB_DEV_IDX_MFC_STR,        /* Index of manufacturer string. */
    USB_DEV_IDX_PRODUCT_STR,    /* Index of product string. */
    USB_DEV_IDX_SERIAL_STR,     /* Index of serial number string. */
    USBD_CFG_MAX_NUM            /* Number of configurations. */
};

/* USB standard language ID descriptor. */
static uint8_t usb_fun_langid_desc[USB_LANGID_DESC_SIZE] __attribute__ ((aligned (4))) =
{
    USB_LANGID_DESC_SIZE,
    USB_DESC_TYPE_STRING,
    LOBYTE(USB_FUN_CDC_LANGID_STR),
    HIBYTE(USB_FUN_CDC_LANGID_STR),
};

/* USB standard configuration descriptor. */
static uint8_t usb_fun_cdc_cfg_desc[USB_CDC_CONFIG_DESC_SIZ] __attribute__ ((aligned (4))) =
{
    /* Configuration Descriptor. */
    0x09,   /* bLength: Configuration Descriptor size. */
    USB_CFG_DESC_TYPE,     /* bDescriptorType: Configuration. */
    USB_CDC_CONFIG_DESC_SIZ,        /* wTotalLength:no of returned bytes. */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface. */
    0x01,   /* bConfigurationValue: Configuration value. */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration. */
    0xC0,   /* bmAttributes: self powered. */
    0x32,   /* MaxPower 0 mA. */

    /*---------------------------------------------------------------------------*/

    /* Interface Descriptor. */
    0x09,   /* bLength: Interface Descriptor size. */
    USB_IFACE_DESC_TYPE,  /* bDescriptorType: Interface. */
    /* Interface descriptor type. */
    0x00,   /* bInterfaceNumber: Number of Interface. */
    0x00,   /* bAlternateSetting: Alternate setting. */
    0x01,   /* bNumEndpoints: One endpoints used. */
    0x02,   /* bInterfaceClass: Communication Interface Class. */
    0x02,   /* bInterfaceSubClass: Abstract Control Model. */
    0x01,   /* bInterfaceProtocol: Common AT commands. */
    0x00,   /* iInterface: */

    /* Header Functional Descriptor */
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,

    /* Call Management Functional Descriptor */
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x01,   /* bDataInterface: 1 */

    /* ACM Functional Descriptor */
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */

    /* Union Functional Descriptor */
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */

    /* Endpoint 2 Descriptor. */
    0x07,                           /* bLength: Endpoint Descriptor size */
    USB_EP_DESC_TYPE,   /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                     /* bEndpointAddress */
    0x03,                           /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SZE),
    0x10,                           /* bInterval: */

    /*---------------------------------------------------------------------------*/

    /* Data class interface descriptor. */
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_IFACE_DESC_TYPE,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */

    /* Endpoint OUT Descriptor. */
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_EP_DESC_TYPE,      /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00,                              /* bInterval: ignore for Bulk transfer */

    /* Endpoint IN Descriptor. */
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_EP_DESC_TYPE,      /* bDescriptorType: Endpoint */
    CDC_IN_EP,                         /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00                               /* bInterval: ignore for Bulk transfer */
};

static uint8_t usb_fun_cdc_other_cfg[USB_CDC_CONFIG_DESC_SIZ] __attribute__ ((aligned (4))) =
{
    0x09,   /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_SPEED_CONFIG,
    USB_CDC_CONFIG_DESC_SIZ,
    0x00,
    0x02,   /* bNumInterfaces: 2 interfaces */
    0x01,   /* bConfigurationValue: */
    0x04,   /* iConfiguration: */
    0xC0,   /* bmAttributes: */
    0x32,   /* MaxPower 100 mA */

    /* Interface Descriptor. */
    0x09,   /* bLength: Interface Descriptor size */
    USB_IFACE_DESC_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */

    /* .Header Functional Descriptor. */
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,

    /* Call Management Functional Descriptor. */
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x01,   /* bDataInterface: 1 */

    /* ACM Functional Descriptor. */
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */

    /* Union Functional Descriptor. */
    0x05,                               /* bFunctionLength */
    0x24,                               /* bDescriptorType: CS_INTERFACE */
    0x06,                               /* bDescriptorSubtype: Union func desc */
    0x00,                               /* bMasterInterface: Communication class interface */
    0x01,                               /* bSlaveInterface0: Data Class Interface */

    /* Endpoint 2 Descriptor. */
    0x07,                               /* bLength: Endpoint Descriptor size */
    USB_EP_DESC_TYPE,                   /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                         /* bEndpointAddress */
    0x03,                               /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SZE),         /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SZE),
    0xFF,                               /* bInterval: */

    /*---------------------------------------------------------------------------*/

    /* Data class interface descriptor. */
    0x09,                               /* bLength: Endpoint Descriptor size */
    USB_IFACE_DESC_TYPE,                /* bDescriptorType: */
    0x01,                               /* bInterfaceNumber: Number of Interface */
    0x00,                               /* bAlternateSetting: Alternate setting */
    0x02,                               /* bNumEndpoints: Two endpoints used */
    0x0A,                               /* bInterfaceClass: CDC */
    0x00,                               /* bInterfaceSubClass: */
    0x00,                               /* bInterfaceProtocol: */
    0x00,                               /* iInterface: */

    /* Endpoint OUT Descriptor. */
    0x07,                               /* bLength: Endpoint Descriptor size */
    USB_EP_DESC_TYPE,                   /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                         /* bEndpointAddress */
    0x02,                               /* bmAttributes: Bulk */
    0x40,                               /* wMaxPacketSize: */
    0x00,
    0x00,                               /* bInterval: ignore for Bulk transfer */

    /* Endpoint IN Descriptor. */
    0x07,                               /* bLength: Endpoint Descriptor size */
    USB_EP_DESC_TYPE,                   /* bDescriptorType: Endpoint */
    CDC_IN_EP,                          /* bEndpointAddress */
    0x02,                               /* bmAttributes: Bulk */
    0x40,                               /* wMaxPacketSize: */
    0x00,
    0x00                                /* bInterval */
};

/* USB CDC function descriptor callbacks. */
static USB_FUN_DESC_CB usb_cdc_desc_cb =
{
    .get_device_desc = &usb_fun_cdc_device_descriptor,
    .get_lang_id_desc = &usb_fun_cdc_langid_descriptor,
    .get_mfg_str_desc = &usb_fun_cdc_manufacturer_descriptor,
    .get_product_str_desc = &usb_fun_cdc_product_descriptor,
    .get_serial_number_str_desc = &usb_fun_cdc_serial_number_descriptor,
    .get_cfg_str_desc = &usb_fun_cdc_config_descriptor,
    .get_iface_str_desc = &usb_fun_cdc_iface_descriptor,
};

/* USB CDC function layer callbacks. */
USB_FUN_CB usb_fun_cdc_cb =
{
    /* Initialization routines. */
    .init = &usb_fun_cdc_init,
    .deinit = &usb_fun_cdc_deinit,

    /* Endpoint management. */
    .setup = &usb_fun_cdc_setup,
    .ep0_rx_ready = &usb_fun_cdc_ep0_rx_ready,
    .data_in = &usb_fun_cdc_data_in,
    .data_out = &usb_fun_cdc_data_out,
    .sof = &usb_fun_cdc_sof,

    /* Configuration API. */
    .get_config_descriptor = &usb_fun_cdc_get_cfg_desc,
    .get_other_config_descriptor = &usb_fun_cdc_get_other_cfg_desc,

    .desc_cb = &usb_cdc_desc_cb,
};

/*
 * usb_fun_cdc_init
 * @usb_device: USB device instance.
 * @cfgidx: Configuration index.
 * Initializes USB CDC layer.
 */
static uint32_t usb_fun_cdc_init(void *usb_device, uint8_t cfgidx)
{
    uint8_t *pbuf;
    extern uint8_t usb_fun_cdc_device_desc[USB_DEVICE_DESC_SIZE];

    /* Remove compiler warning. */
    UNUSED_PARAM(cfgidx);

    /* Open IN endpoint. */
    usb_fun_endpoint_open(usb_device, CDC_IN_EP, *(uint16_t *)(((USB_STM32F407_HANDLE *)usb_device)->device.config_desc + 57), EP_TYPE_BULK);

    /* Open OUT endpoint. */
    usb_fun_endpoint_open(usb_device, CDC_OUT_EP, *(uint16_t *)(((USB_STM32F407_HANDLE *)usb_device)->device.config_desc + 64), EP_TYPE_BULK);

    /* Open CMD endpoint. */
    usb_fun_endpoint_open(usb_device, CDC_CMD_EP, CDC_CMD_PACKET_SZE, EP_TYPE_INTR);

    pbuf = (uint8_t *)usb_fun_cdc_device_desc;
    pbuf[4] = DEVICE_CLASS_CDC;
    pbuf[5] = DEVICE_SUBCLASS_CDC;

    /* Initialize console data. */
    ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd = NO_CMD;
    ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.altset = 0;

    /* Prepare OUT endpoint to receive incoming packet. */
    usb_fun_endpoint_prepare_rx(usb_device, CDC_OUT_EP, (uint8_t*)(((USB_FUN_CDC_DEV *)usb_device)->cdc_console.rx_buffer), *(uint16_t *)(((USB_STM32F407_HANDLE *)usb_device)->device.config_desc + 64));

    /* Return success. */
    return (SUCCESS);

} /* usb_fun_cdc_init */

/*
 * usb_fun_cdc_deinit
 * @usb_device: USB device instance.
 * @cfgidx: Configuration index.
 * Deinitialize CDC layer.
 */
static uint32_t usb_fun_cdc_deinit(void *usb_device, uint8_t cfgidx)
{
    /* Remove compiler warning. */
    UNUSED_PARAM(cfgidx);

    /* Close IN endpoint. */
    usb_fun_endpoint_close(usb_device, CDC_IN_EP);

    /* Close OUT endpoint. */
    usb_fun_endpoint_close(usb_device, CDC_OUT_EP);

    /* Close CMD endpoint. */
    usb_fun_endpoint_close(usb_device, CDC_CMD_EP);

    /* Return success. */
    return (SUCCESS);

} /* usb_fun_cdc_deinit */

/*
 * usb_fun_cdc_setup
 * @usb_device: USB device instance.
 * @req: USB request needed to be setup.
 * Handle the CDC specific requests.
 */
static uint32_t usb_fun_cdc_setup(void *usb_device, USB_SETUP_REQ *req)
{
    uint16_t len;
    uint8_t *pbuf, ret = SUCCESS;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    case USB_REQ_TYPE_CLASS:

        /* Check if the request is a data setup packet. */
        if (req->wLength)
        {
            /* Check if the request is device-to-host. */
            if (req->bmRequest & 0x80)
            {
                /* Get the data to be sent to host from interface layer. */
                usb_cdc_fun_console_handle_ctrl(usb_device, req->bRequest, ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd_buffer, req->wLength);

                /* Send the data to the host. */
                usb_fun_control_tx(usb_device, (uint8_t *)((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd_buffer, req->wLength);
            }

            /* Host-to-device request. */
            else
            {
                /* Set the value of the current command to be processed. */
                ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd = req->bRequest;
                ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd_len = req->wLength;

                /* Prepare the reception of the buffer over endpoint0.
                 * Next step: the received data will be managed in ep0_tx_sent
                 * function. */
                usb_fun_control_rx(usb_device, (uint8_t *)((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd_buffer, req->wLength);
            }
        }

        /* No data request. */
        else
        {
            /* Transfer the command to the interface layer. */
            usb_cdc_fun_console_handle_ctrl(usb_device, req->bRequest, NULL, 0);
        }

        break;

    case USB_REQ_TYPE_STANDARD:

        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:

            if( (req->wValue >> 8) == CDC_DESCRIPTOR_TYPE)
            {
#ifdef STM32F407_USB_HS_DMA_ENABLED
                pbuf = usb_fun_cdc_cfg_desc;
#else
                pbuf = usb_fun_cdc_cfg_desc + 9 + (9 * USBD_ITF_MAX_NUM);
#endif
                len = MIN(USB_CDC_DESC_SIZ , req->wLength);
            }

            usb_fun_control_tx (usb_device, pbuf, len);
            break;

        case USB_REQ_GET_INTERFACE:
            usb_fun_control_tx(usb_device, (uint8_t *)&((USB_FUN_CDC_DEV *)usb_device)->cdc_console.altset, 1);
            break;

        case USB_REQ_SET_INTERFACE:
            if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
            {
                ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.altset = (uint8_t)(req->wValue);
            }
            else
            {
                /* Call the error management function. */
                usb_fun_control_error(usb_device, req);
            }
            break;
        }

        break;

    default:

        /* Call the error management function. */
        usb_fun_control_error(usb_device, req);
        ret = 1;
    }

    /* Return status code. */
    return (ret);

} /* usb_fun_cdc_setup */

/*
 * usb_fun_cdc_ep0_rx_ready
 * @usb_device: USB device instance.
 * Handles event when data is received on control endpoint.
 */
static uint32_t usb_fun_cdc_ep0_rx_ready(void *usb_device)
{
    if (((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd != NO_CMD)
    {
        /* Process this command. */
        usb_cdc_fun_console_handle_ctrl(&((USB_FUN_CDC_DEV *)usb_device)->cdc_console, ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd, ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd_buffer, (int32_t)((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd_len);

        /* Reset the command variable to default value. */
        ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.cmd = NO_CMD;
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_fun_cdc_ep0_rx_ready */

/*
 * usb_fun_cdc_data_in
 * @usb_device: USB device instance.
 * @epnum: Endpoint number.
 * Handles data TX a non-control endpoint.
 */
static uint32_t usb_fun_cdc_data_in(void *usb_device, uint8_t epnum)
{
    int32_t tx_length;

    /* Remove compiler warning. */
    UNUSED_PARAM(epnum);

    /* Check if there is some data available to be sent on the console. */
    tx_length = usb_cdc_fun_console_handle_tx(&((USB_FUN_CDC_DEV *)usb_device)->cdc_console, ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.tx_buffer, CDC_DATA_MAX_PACKET_SIZE);

    if (tx_length > 0)
    {
        /* Prepare the available data buffer to be sent on IN endpoint. */
        usb_fun_endpoint_tx(usb_device, CDC_IN_EP, (uint8_t*)(((USB_FUN_CDC_DEV *)usb_device)->cdc_console.tx_buffer), (uint32_t)tx_length);
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_fun_cdc_data_in */

/*
 * usb_fun_cdc_data_out
 * @usb_device: USB device instance.
 * @epnum: Endpoint number on which data was received.
 * Handles data RX on non control endpoint.
 */
static uint32_t usb_fun_cdc_data_out(void *usb_device, uint8_t epnum)
{
    /* Handle RX on CDC device. */
    usb_cdc_fun_console_handle_rx(&((USB_FUN_CDC_DEV *)usb_device)->cdc_console, ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.rx_buffer, (int32_t)((USB_STM32F407_HANDLE *)usb_device)->device.out_ep[epnum].xfer_count);

    /* Prepare out endpoint to receive next packet. */
    usb_fun_endpoint_prepare_rx(usb_device, CDC_OUT_EP, (uint8_t*)(((USB_FUN_CDC_DEV *)usb_device)->cdc_console.rx_buffer), *(uint16_t *)(((USB_STM32F407_HANDLE *)usb_device)->device.config_desc + 64));

    /* Return success. */
    return (SUCCESS);

} /* usb_fun_cdc_data_out */

/*
 * usb_fun_cdc_sof
 * @usb_device: USB device instance.
 * Handles start of a frame.
 */
static uint32_t usb_fun_cdc_sof(void *usb_device)
{
    int32_t tx_length;

    /* Check if there is some data available to be sent on the console. */
    tx_length = usb_cdc_fun_console_handle_tx(&((USB_FUN_CDC_DEV *)usb_device)->cdc_console, ((USB_FUN_CDC_DEV *)usb_device)->cdc_console.tx_buffer, CDC_DATA_MAX_PACKET_SIZE);

    if (tx_length > 0)
    {
        /* Prepare the available data buffer to be sent on IN endpoint. */
        usb_fun_endpoint_tx(usb_device, CDC_IN_EP, (uint8_t*)(((USB_FUN_CDC_DEV *)usb_device)->cdc_console.tx_buffer), (uint32_t)tx_length);
    }

    /* Return success. */
    return (SUCCESS);

} /* usb_fun_cdc_sof */

/*
 * usb_fun_cdc_get_cfg_desc
 * @speed: Current device speed.
 * @length: Pointer where data length will be returned.
 * Returns configuration descriptor.
 */
static uint8_t *usb_fun_cdc_get_cfg_desc(uint8_t speed, uint16_t *length)
{
    /* Remove compiler warning. */
    UNUSED_PARAM(speed);

    *length = sizeof (usb_fun_cdc_cfg_desc);

    return (usb_fun_cdc_cfg_desc);

} /* usb_fun_cdc_get_cfg_desc */

/*
 * usb_fun_cdc_get_other_cfg_desc
 * @speed: Current device speed.
 * @length: Pointer where data length will be returned.
 * Returns configuration descriptor.
 */
static uint8_t *usb_fun_cdc_get_other_cfg_desc(uint8_t speed, uint16_t *length)
{
    /* Remove compiler warning. */
    UNUSED_PARAM(speed);

    *length = sizeof (usb_fun_cdc_other_cfg);

    return (usb_fun_cdc_other_cfg);

} /* usb_fun_cdc_get_other_cfg_desc */

/*
 * usb_fun_cdc_device_descriptor
 * @speed: Current device speed.
 * @length: Pointer where data length will be returned.
 * Returns device descriptor.
 */
uint8_t *usb_fun_cdc_device_descriptor(uint8_t speed, uint16_t *length)
{
    UNUSED_PARAM(speed);

    /* Return the length of the descriptor that will be returned. */
    *length = sizeof(usb_fun_cdc_device_desc);

    /* Return the descriptor. */
    return (usb_fun_cdc_device_desc);

} /* usb_fun_cdc_device_descriptor */

/*
 * usb_fun_cdc_langid_descriptor
 * @speed: Current device speed.
 * @length: Pointer where data length will be returned.
 * Returns language ID descriptor.
 */
uint8_t *usb_fun_cdc_langid_descriptor(uint8_t speed, uint16_t *length)
{
    UNUSED_PARAM(speed);

    /* Return the length of the descriptor that will be returned. */
    *length = sizeof(usb_fun_langid_desc);

    /* Return the descriptor. */
    return (usb_fun_langid_desc);

} /* usb_fun_cdc_langid_descriptor */

/*
 * usb_fun_cdc_product_descriptor
 * @speed: Current device speed.
 * @desc_buffer: Descriptor buffer that can be used to store the required string.
 * @length: Pointer where data length will be returned, on input will contain
 *  the size of provided buffer.
 * Returns the product descriptor.
 */
uint8_t *usb_fun_cdc_product_descriptor(uint8_t speed, uint8_t *desc_buffer, uint16_t *length)
{
    /* Convert the required descriptor. */
    if (speed == USB_STM32F407_SPEED_HIGH)
    {
        usb_fun_get_string((uint8_t*)USB_FUN_CDC_PDT_HS_STR, desc_buffer, length);
    }
    else
    {
        usb_fun_get_string((uint8_t*)USB_FUN_CDC_PDT_FS_STR, desc_buffer, length);
    }

    /* Return the same descriptor buffer. */
    return (desc_buffer);

} /* usb_fun_cdc_product_descriptor */

/*
 * usb_fun_cdc_manufacturer_descriptor
 * @speed: Current device speed.
 * @desc_buffer: Descriptor buffer that can be used to store the required string.
 * @length: Pointer where data length will be returned, on input will contain
 *  the size of provided buffer.
 * Returns the manufacturer descriptor.
 */
uint8_t *usb_fun_cdc_manufacturer_descriptor(uint8_t speed, uint8_t *desc_buffer, uint16_t *length)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(speed);

    /* Convert the required descriptor. */
    usb_fun_get_string((uint8_t*)USB_FUN_CDC_MFGR_STR, desc_buffer, length);

    /* Return the same descriptor buffer. */
    return (desc_buffer);

} /* usb_fun_cdc_manufacturer_descriptor */

/*
 * usb_fun_cdc_serial_number_descriptor
 * @speed: Current device speed.
 * @desc_buffer: Descriptor buffer that can be used to store the required string.
 * @length: Pointer where data length will be returned, on input will contain
 *  the size of provided buffer.
 * Returns the serial number descriptor.
 */
uint8_t *usb_fun_cdc_serial_number_descriptor(uint8_t speed, uint8_t *desc_buffer, uint16_t *length)
{
    /* Convert the required descriptor. */
    if (speed == USB_STM32F407_SPEED_HIGH)
    {
        usb_fun_get_string((uint8_t*)USB_FUN_CDC_SN_HS_STR, desc_buffer, length);
    }
    else
    {
        usb_fun_get_string((uint8_t*)USB_FUN_CDC_SN_FS_STR, desc_buffer, length);
    }

    /* Return the same descriptor buffer. */
    return (desc_buffer);

} /* usb_fun_cdc_serial_number_descriptor */

/*
 * usb_fun_cdc_config_descriptor
 * @speed: Current device speed.
 * @desc_buffer: Descriptor buffer that can be used to store the required string.
 * @length: Pointer where data length will be returned, on input will contain
 *  the size of provided buffer.
 * Returns the configuration string descriptor.
 */
uint8_t *usb_fun_cdc_config_descriptor(uint8_t speed, uint8_t *desc_buffer, uint16_t *length)
{
    /* Convert the required descriptor. */
    if (speed == USB_STM32F407_SPEED_HIGH)
    {
        usb_fun_get_string((uint8_t*)USB_FUN_CDC_CFG_HS_STR, desc_buffer, length);
    }
    else
    {
        usb_fun_get_string((uint8_t*)USB_FUN_CDC_CFG_FS_STR, desc_buffer, length);
    }

    /* Return the same descriptor buffer. */
    return (desc_buffer);

} /* usb_fun_cdc_config_descriptor */

/*
 * usb_fun_cdc_iface_descriptor
 * @speed: Current device speed.
 * @desc_buffer: Descriptor buffer that can be used to store the required string.
 * @length: Pointer where data length will be returned, on input will contain
 *  the size of provided buffer.
 * Returns the interface string descriptor.
 */
uint8_t *usb_fun_cdc_iface_descriptor(uint8_t speed, uint8_t *desc_buffer, uint16_t *length)
{
    /* Convert the required descriptor. */
    if (speed == USB_STM32F407_SPEED_HIGH)
    {
        usb_fun_get_string ((uint8_t*)USB_FUN_CDC_IFACE_HS_STR, desc_buffer, length);
    }
    else
    {
        usb_fun_get_string ((uint8_t*)USB_FUN_CDC_IFACE_FS_STR, desc_buffer, length);
    }

    /* Return the same descriptor buffer. */
    return (desc_buffer);

} /* usb_fun_cdc_iface_descriptor */
#endif /* USB_FUNCTION */
