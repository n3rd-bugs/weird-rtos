/*
 * usb_cdc_console.c
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
#include <sll.h>

#ifdef USB_CDC_CONSOLE

/* Internal function prototypes. */
static int32_t usb_cdc_fun_console_read(void *, char *, int32_t);
static int32_t usb_cdc_fun_console_write(void *, char *, int32_t);
static void usb_cdc_fun_console_rx_consumed(void *, void *);
static void usb_cdc_fun_console_space_available(void *, void *);

/*
 * usb_cdc_console_register
 * @cdc_cons: CDC console instance needed to be initialized.
 * @usb_device: USB device handle to use.
 * This function will register a USB CDC console.
 */
void usb_cdc_console_register(CDC_CONSOLE *cdc_cons, void *usb_device)
{
    uint32_t i;

    /* Note: this should handle both device and host consoles. */
    /* For function console. */

    /* Assign console device. */
    cdc_cons->usb_device = usb_device;

    /* Register this console with the file system. */
    cdc_cons->console.fs.read = usb_cdc_fun_console_read;
    cdc_cons->console.fs.write = usb_cdc_fun_console_write;
    cdc_cons->console.fs.rx_consumed = usb_cdc_fun_console_rx_consumed;
    console_register(&cdc_cons->console);

    cdc_cons->data_watcher.data = &(cdc_cons->console);
    cdc_cons->data_watcher.space_available = usb_cdc_fun_console_space_available;
    cdc_cons->data_watcher.data_available = NULL;
    fs_set_data_watcher(&cdc_cons->console, &cdc_cons->data_watcher);

    /* Add buffer for this console. */
    for (i = 0; i < CDC_NUM_BUFFERS; i++)
    {
        /* Initialize a buffer. */
        fs_init_buffer(&cdc_cons->fs_buffer[i], &cdc_cons->buffer[CDC_DATA_MAX_PACKET_SIZE * i], CDC_DATA_MAX_PACKET_SIZE);

        /* Add this buffer to the free buffer list for this file descriptor. */
        fs_add_buffer((FD)&cdc_cons->console, &cdc_cons->fs_buffer[i], FS_FREE_BUFFER);
    }

    /* This will block on read, and all data that will be given to write must
     * be flushed. */
    cdc_cons->console.fs.flags = (FS_BLOCK | FS_BUFFERED | FS_FLUSH_WRITE |
                                  FS_SPACE_AVAILABLE);

} /* usb_cdc_console_register */

/*
 * usb_cdc_console_unregister
 * @cdc_cons: CDC console needed to be unregistered.
 * This function will register a USB CDC console.
 */
void usb_cdc_console_unregister(CDC_CONSOLE *cdc_cons)
{
    /* Unregister this console with the file system. */
    console_unregister(&cdc_cons->console);

} /* usb_cdc_console_unregister */

/*
 * usb_cdc_console_handle_connect
 * @cdc_cons: CDC console connected.
 * This function will be called when USB device is connected.
 */
void usb_cdc_console_handle_connect(CDC_CONSOLE *cdc_cons)
{
    /* Handle the connection event. */
    fs_connected((FD)cdc_cons);

} /* usb_cdc_console_handle_connect */

/*
 * usb_cdc_console_handle_disconnect
 * @cdc_cons: CDC console disconnected.
 * This function will be called when USB device is disconnected.
 */
void usb_cdc_console_handle_disconnect(CDC_CONSOLE *cdc_cons)
{
    /* Handle the disconnection event. */
    fs_disconnected((FD)cdc_cons);

} /* usb_cdc_console_handle_disconnect */

/*
 * usb_cdc_fun_console_handle_rx
 * @cdc_cons: USB CDC function console context.
 * @nbytes: Number of bytes received.
 * This function will be called whenever there is some data available on a CDC
 * console device.
 */
void usb_cdc_fun_console_handle_rx(CDC_CONSOLE *cdc_cons, uint32_t nbytes)
{
    FS_BUFFER *buffer;

    /* We are in interrupt so just try to obtain semaphore here. */
    if (cdc_cons->console.fs.get_lock((FD)(&cdc_cons->console)) == SUCCESS)
    {
        /* Update buffer pointers. */
        fs_update_buffer(cdc_cons->rx_buffer, cdc_cons->rx_buffer->buffer, nbytes);

        /* Save and clear the receive buffer. */
        buffer = cdc_cons->rx_buffer;
        cdc_cons->rx_buffer = NULL;

        /* Push this buffer on the RX buffer list. */
        fs_add_buffer((FD)(&cdc_cons->console), buffer, FS_RX_BUFFER);

        /* Release lock. */
        cdc_cons->console.fs.release_lock((FD)(&cdc_cons->console));
    }

} /* usb_cdc_fun_console_handle_rx */

/*
 * usb_cdc_fun_console_handle_tx_complete
 * @cdc_cons: USB CDC function console context.
 * This function will be called when an previously started TX completes.
 */
void usb_cdc_fun_console_handle_tx_complete(CDC_CONSOLE *cdc_cons)
{
    FS_BUFFER *buffer;

    /* We are in interrupt so just try to obtain semaphore here. */
    if (cdc_cons->console.fs.get_lock(&cdc_cons->console) == SUCCESS)
    {
        /* If we were actually transferring some data. */
        if (cdc_cons->tx_buffer)
        {
            /* Save and clear the transmit buffer. */
            buffer = cdc_cons->tx_buffer;
            cdc_cons->tx_buffer = NULL;

            /* Push this buffer back to the free list. */
            fs_add_buffer((FD)(&cdc_cons->console), buffer, FS_FREE_BUFFER);
        }

        /* Release lock. */
        cdc_cons->console.fs.release_lock(&cdc_cons->console);
    }

} /* usb_cdc_fun_console_handle_tx_complete */

/*
 * usb_cdc_fun_console_handle_tx
 * @cdc_cons: USB CDC function console context.
 * @return: Returns a transmit buffer if available.
 * This function will be called whenever an SOF is received. This
 * function will see if we need to send some data and copy the required data
 * in the given buffer.
 */
FS_BUFFER *usb_cdc_fun_console_handle_tx(CDC_CONSOLE *cdc_cons)
{
    FS_BUFFER *buffer = NULL;

    /* We are in interrupt so just try to obtain semaphore here. */
    if (cdc_cons->console.fs.get_lock(&cdc_cons->console) == SUCCESS)
    {
        /* Check if we have something to transmit. */
        buffer = cdc_cons->tx_buffer = fs_get_buffer(((FD)&cdc_cons->console), FS_TX_BUFFER);

        /* Release lock. */
        cdc_cons->console.fs.release_lock(&cdc_cons->console);
    }

    /* Return buffer that can be sent on the console. */
    return (buffer);

} /* usb_cdc_fun_console_handle_tx */

/*
 * usb_cdc_fun_console_handle_ctrl
 * @cdc_cons: USB CDC function console context.
 * @cmd: Command needed to be processed.
 * @buf: Command data buffer.
 * @nbytes: Number of bytes that can be copied or given.
 * This function will be called whenever a CTRL event occur.
 */
void usb_cdc_fun_console_handle_ctrl(CDC_CONSOLE *cdc_cons, uint32_t cmd, char *buf, int32_t nbytes)
{
    /* For now does nothing. */
    UNUSED_PARAM(cdc_cons);
    UNUSED_PARAM(cmd);
    UNUSED_PARAM(buf);
    UNUSED_PARAM(nbytes);

} /* usb_cdc_fun_console_handle_ctrl */

/*
 * usb_cdc_fun_console_rx_consumed
 * @fd: File descriptor.
 * @buffer: Receive buffer that was consumed.
 * This function will be called when buffer scheme is used and we have
 * consumed the RX buffer.
 */
static void usb_cdc_fun_console_rx_consumed(void *fd, void *buffer)
{
    CDC_CONSOLE *cdc = (CDC_CONSOLE *)fd;

    /* If we actually have consumed some data. */
    if (buffer)
    {
        /* Push this buffer back to the free list. */
        fs_add_buffer((FD)(&cdc->console), (FS_BUFFER *)buffer, FS_FREE_BUFFER);
    }

} /* usb_cdc_fun_console_rx_consumed */

/*
 * usb_cdc_fun_console_read
 * @fd: File descriptor.
 * @buffer: Buffer in which data will be read.
 * @size: Size of buffer.
 * @return: Number of bytes read from CDC console.
 * This function will read data from a USB CDC console.
 */
static int32_t usb_cdc_fun_console_read(void *fd, char *buffer, int32_t size)
{
    CDC_CONSOLE *cdc = (CDC_CONSOLE *)fd;
    FS_BUFFER *fs_buffer = fs_get_buffer(((FD)&cdc->console), FS_RX_BUFFER);

    /* If we do have received a buffer. */
    if (fs_buffer)
    {
        /* Calculate number of bytes that can be copied in the buffer. */
        if (size > (int32_t)fs_buffer->length)
        {
            size = (int32_t)fs_buffer->length;
        }

        /* If we actually have some data. */
        if (fs_buffer->length > 0)
        {
            /* Copy received data from the buffer. */
            memcpy(buffer, fs_buffer->buffer, (uint32_t)size);
        }

        /* This receive buffer is now consumed. */
        usb_cdc_fun_console_rx_consumed(fd, fs_buffer);
    }

    /* Return number of bytes. */
    return (size);

} /* usb_cdc_fun_console_read */

/*
 * usb_cdc_fun_console_space_available
 * @fd: File descriptor.
 * @buffer: Receive buffer that was consumed.
 * This function will be called when buffer scheme is used and we have
 * consumed the RX buffer.
 */
static void usb_cdc_fun_console_space_available(void *fd, void *priv_data)
{
    CDC_CONSOLE *cdc = (CDC_CONSOLE *)fd;

    /* Remove some compiler warnings. */
    UNUSED_PARAM(priv_data);

    /* If we are not receiving any data, start receiving new data. */
    if (cdc->rx_buffer == NULL)
    {
        /* Pick a free buffer. */
        cdc->rx_buffer = fs_get_buffer(fd, FS_FREE_BUFFER);

        if (cdc->rx_buffer != NULL)
        {
            /* Start receiving new data. */
            usb_fun_cdc_acm_data_out_enable(cdc->usb_device);
        }
    }

} /* usb_cdc_fun_console_space_available */

/*
 * usb_cdc_fun_console_write
 * @fd: File descriptor.
 * @buffer: Buffer from which data is needed to be copied.
 * @size: Number of bytes to copy from the buffer.
 * @return: Number of bytes sent on the console.
 * This function will write data on given USB CDC console.
 */
static int32_t usb_cdc_fun_console_write(void *fd, char *buffer, int32_t size)
{
    CDC_CONSOLE *cdc = (CDC_CONSOLE *)fd;
    FS_BUFFER *fs_buffer = fs_get_buffer(((FD)&cdc->console), FS_FREE_BUFFER);

    /* If we do have a free buffer that can be used to transmit this data. */
    if (fs_buffer)
    {
        /* Check if we need to send more bytes than we can send in a single packet. */
        if (size > (int32_t)fs_buffer->length)
        {
            /* Send bytes that can be sent in a packet. */
            size = (int32_t)fs_buffer->length;
        }

        if (size > 0)
        {
            /* Copy data in the TX buffer. */
            memcpy(fs_buffer->buffer, buffer, (uint32_t)size);

            /* Update buffer pointers. */
            fs_update_buffer(fs_buffer, fs_buffer->buffer, (uint32_t)size);

            /* Push this buffer back to the transmit list. */
            fs_add_buffer((FD)(&cdc->console), fs_buffer, FS_TX_BUFFER);
        }
    }

    /* Return number of bytes. */
    return (size);

} /* usb_cdc_fun_console_write */

#endif /* USB_CDC_CONSOLE */