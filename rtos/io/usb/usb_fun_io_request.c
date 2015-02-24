/*
 * usb_fun_io_request.c
 *
 * Copyright(c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 *(in any form) the author will not be liable for any legal charges.
 */
#include <os.h>

#ifdef USB_FUNCTION

/*
 * usb_fun_control_tx
 * @usb_device: USB device instance.
 * @buffer: Buffer from which data is needed to be sent.
 * @length: Length of data in buffer.
 * Sends data on control endpoint.
 */
uint32_t usb_fun_control_tx(USB_STM32F407_HANDLE *usb_device, uint8_t *buffer, uint32_t length)
{

    /* Initialize control IN endpoint to send data. */
    usb_device->device.in_ep[0].total_data_len = length;
    usb_device->device.in_ep[0].rem_data_len = length;
    usb_device->device.state = USB_STM32F407_EP0_DATA_IN;

    /* Transmit control endpoint. */
    usb_fun_endpoint_tx(usb_device, 0, buffer, length);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_control_tx */

/*
 * usb_fun_control_continue_tx
 * @usb_device: USB device instance.
 * @buffer: Buffer from which data is needed to be sent.
 * @length: Length of data in buffer.
 * Continue sending data on control endpoint.
 */
uint32_t usb_fun_control_continue_tx(USB_STM32F407_HANDLE *usb_device, uint8_t *buffer, uint32_t length)
{
    /* Transmit control endpoint. */
    usb_fun_endpoint_tx(usb_device, 0, buffer, length);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_control_continue_tx */

/*
 * usb_fun_control_rx
 * @usb_device: USB device instance.
 * @buffer: Buffer in which data is needed to be received.
 * @length: Maximum number of bytes that can copied.
 * Receives data on control endpoint.
 */
uint32_t usb_fun_control_rx(USB_STM32F407_HANDLE *usb_device, uint8_t *buffer, uint32_t length)
{
    /* Initialize OUT control endpoint. */
    usb_device->device.out_ep[0].total_data_len = length;
    usb_device->device.out_ep[0].rem_data_len   = length;
    usb_device->device.state = USB_STM32F407_EP0_DATA_OUT;

    /* Receive data from control endpoint. */
    usb_fun_endpoint_prepare_rx(usb_device, 0, buffer, length);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_control_rx */

/*
 * usb_fun_control_continue_rx
 * @usb_device: USB device instance.
 * @buffer: Buffer in which data is needed to be received.
 * @length: Maximum number of bytes that can copied.
 * Continue receiving data on control endpoint.
 */
uint32_t usb_fun_control_continue_rx(USB_STM32F407_HANDLE *usb_device, uint8_t *buffer, uint32_t length)
{
    /* Receive data from control endpoint. */
    usb_fun_endpoint_prepare_rx(usb_device, 0, buffer, length);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_control_continue_rx */

/*
 * usb_fun_control_send_status
 * @usb_device: USB device instance.
 * Send a zero length packet on control endpoint.
 */
uint32_t usb_fun_control_send_status(USB_STM32F407_HANDLE *usb_device)
{
    /* Update device status. */
    usb_device->device.state = USB_STM32F407_EP0_STATUS_IN;

    /* Transmit a packet on control endpoint. */
    usb_fun_endpoint_tx(usb_device, 0, NULL, 0);

    /* Start endpoint transmission. */
    usb_function_stm32f407_ep0_start_out(usb_device);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_control_send_status */

/*
 * usb_fun_control_receive_status
 * @usb_device: USB device instance.
 * Receives a zero length packet on control endpoint.
 */
uint32_t usb_fun_control_receive_status(USB_STM32F407_HANDLE *usb_device)
{
    /* Update device status. */
    usb_device->device.state = USB_STM32F407_EP0_STATUS_OUT;

    /* Receive a packet on control endpoint. */
    usb_fun_endpoint_prepare_rx(usb_device, 0, NULL, 0);

    /* Start endpoint transmission. */
    usb_function_stm32f407_ep0_start_out(usb_device);

    /* Always return success. */
    return (SUCCESS);

} /* usb_fun_control_receive_status */


/*
 * usb_fun_get_rx_count
 * @usb_device: USB device instance.
 * @endpoint: Endpoint from on which length is needed.
 * Returns total number of bytes received on an endpoint.
 */
uint32_t usb_fun_get_rx_count(USB_STM32F407_HANDLE *usb_device, uint8_t endpoint)
{
    /* Return RX count for this endpoint. */
    return (usb_device->device.out_ep[endpoint].xfer_count);

} /* usb_fun_get_rx_count */

#endif /* USB_FUNCTION */
