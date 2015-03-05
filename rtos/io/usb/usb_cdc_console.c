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

#ifdef USB_CDC_CONSOLE

/* Internal function prototypes. */
static int32_t usb_cdc_fun_console_read(void *, char *, int32_t);
static int32_t usb_cdc_fun_console_write(void *, char *, int32_t);
static void usb_cdc_fun_console_rx_consumed(void *);
static void usb_cdc_fun_console_tx_available(void *);

/*
 * usb_cdc_console_register
 * This function will register a USB CDC console.
 */
void usb_cdc_console_register(CDC_CONSOLE *cdc_cons)
{
    /* Note: this should handle both device and host consoles. */
    /* For function console. */

    /* Register this console with the file system. */
    cdc_cons->console.fs.read = usb_cdc_fun_console_read;
    cdc_cons->console.fs.write = usb_cdc_fun_console_write;
    cdc_cons->console.fs.tx_buffer = cdc_cons->tx_buffer;
    cdc_cons->console.fs.rx_buffer = cdc_cons->rx_buffer;
    cdc_cons->console.fs.tx_len = CDC_DATA_MAX_PACKET_SIZE;
    cdc_cons->console.fs.rx_len = 0;
    cdc_cons->console.fs.rx_consumed = usb_cdc_fun_console_rx_consumed;
    cdc_cons->console.fs.tx_available = usb_cdc_fun_console_tx_available;
    console_register(&cdc_cons->console);

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
 * usb_cdc_fun_console_handle_rx
 * @cdc_cons: USB CDC function console context.
 * @nbytes: Number of bytes received.
 * This function will be called whenever there is some data available on a CDC
 * console device.
 */
void usb_cdc_fun_console_handle_rx(CDC_CONSOLE *cdc_cons, uint32_t nbytes)
{
    /* We are in interrupt so just try to obtain semaphore here. */
    if (cdc_cons->console.fs.get_lock(&cdc_cons->console) == SUCCESS)
    {
        /* Save number of bytes available. */
        cdc_cons->console.fs.rx_len = cdc_cons->rx_len = nbytes;

        /* Tell upper layer that some data is now available. */
        fd_data_available(&cdc_cons->console);

        /* Release lock. */
        cdc_cons->console.fs.release_lock(&cdc_cons->console);
    }

} /* usb_cdc_fun_console_handle_rx */

/*
 * usb_cdc_fun_console_handle_tx_complete
 * @cdc_cons: USB CDC function console context.
 * This function will be called when an previously started TX completes.
 */
void usb_cdc_fun_console_handle_tx_complete(CDC_CONSOLE *cdc_cons)
{
    /* We are in interrupt so just try to obtain semaphore here. */
    if (cdc_cons->console.fs.get_lock(&cdc_cons->console) == SUCCESS)
    {
        /* If we were actually transferring some data. */
        if (cdc_cons->console.fs.tx_len == 0)
        {
            /* Reset the number of bytes to be sent. */
            cdc_cons->tx_len = 0;

            /* Recover the number of bytes that can be written on this FD. */
            cdc_cons->console.fs.tx_len = CDC_DATA_MAX_PACKET_SIZE;

            /* Tell upper layer that some space is now available. */
            fd_space_available(cdc_cons);
        }

        /* Release lock. */
        cdc_cons->console.fs.release_lock(&cdc_cons->console);
    }

} /* usb_cdc_fun_console_handle_tx_complete */

/*
 * usb_cdc_fun_console_handle_tx
 * @cdc_cons: USB CDC function console context.
 * This function will be called whenever an SOF is received. This
 * function will see if we need to send some data and copy the required data
 * in the given buffer.
 */
uint32_t usb_cdc_fun_console_handle_tx(CDC_CONSOLE *cdc_cons)
{
    uint32_t nbytes = 0;

    /* We are in interrupt so just try to obtain semaphore here. */
    if (cdc_cons->console.fs.get_lock(&cdc_cons->console) == SUCCESS)
    {
        /* If we do have to copy some data. */
        if (cdc_cons->tx_len > 0)
        {
            /* Save number of bytes that are needed to be sent. */
            nbytes = cdc_cons->tx_len;
        }

        /* Release lock. */
        cdc_cons->console.fs.release_lock(&cdc_cons->console);
    }

    /* Return number of bytes that can be sent on the console. */
    return (nbytes);

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
 * This function will be called when buffer scheme is used and we have
 * consumed the RX buffer.
 */
static void usb_cdc_fun_console_rx_consumed(void *fd)
{
    CDC_CONSOLE *cdc = (CDC_CONSOLE *)fd;

    /* If we actually have some data. */
    if (cdc->rx_len > 0)
    {
        /* TODO: Will discard any remaining data. */
        cdc->console.fs.rx_len = cdc->rx_len = 0;

        /* Receive remaining data from the device. */
        usb_fun_cdc_acm_data_out_enable(cdc->usb_device);
    }

    /* We will be receiving one packet at a time so tell the file system that
     * there is no more available to read. */
    fd_data_flushed(fd);

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

    /* Calculate number of bytes that can be copied in the buffer. */
    if (size > (int32_t)cdc->rx_len)
    {
        size = (int32_t)cdc->rx_len;
    }

    /* If we actually have some data. */
    if (cdc->rx_len > 0)
    {
        /* Copy received data from the buffer. */
        memcpy(buffer, cdc->rx_buffer, (uint32_t)size);
    }

    /* We will be receiving one packet at a time so tell the file system that
     * there is no more data available to read. */
    usb_cdc_fun_console_rx_consumed(fd);

    /* Return number of bytes. */
    return (size);

} /* usb_cdc_fun_console_read */

/*
 * usb_cdc_fun_console_tx_available
 * @fd: File descriptor.
 * This function will be called when we have some data to transfer when
 * using buffered scheme.
 */
static void usb_cdc_fun_console_tx_available(void *fd)
{
    CDC_CONSOLE *cdc = (CDC_CONSOLE *)fd;

    /* Tell the file system to block the write until there is some
     * space available. */
    fd_space_consumed(fd);

    /* Save number of bytes that can be sent on the USB device. */
    cdc->tx_len = cdc->console.fs.tx_len;

    /* No more data can be pushed now. */
    cdc->console.fs.tx_len = 0;

} /* usb_cdc_fun_console_tx_available */

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

    /* Check if we need to send more bytes than we can send in a single packet. */
    if (size > CDC_DATA_MAX_PACKET_SIZE)
    {
        /* Send bytes that can be sent in a packet. */
        size = CDC_DATA_MAX_PACKET_SIZE;
    }

    if (size > 0)
    {
        /* Copy data in the TX buffer. */
        memcpy(cdc->tx_buffer, buffer, (uint32_t)size);

        /* Save number of bytes needed to be sent. */
        cdc->console.fs.tx_len = (uint32_t)size;

        /* Enable TX for the file descriptor. */
        usb_cdc_fun_console_tx_available(fd);
    }

    /* Return number of bytes. */
    return (size);

} /* usb_cdc_fun_console_write */

#endif /* USB_CDC_CONSOLE */
