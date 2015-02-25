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

#ifdef USB_CDC_CONSOLE

/* Internal function prototypes. */
static int32_t usb_cdc_fun_space_available(void *);
static int32_t usb_cdc_fun_console_read(void *, char *, int32_t);
static int32_t usb_cdc_fun_console_write(void *, char *, int32_t);

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
    cdc_cons->console.fs.space_available = usb_cdc_fun_space_available;
    console_register(&cdc_cons->console);

    /* This will block on read, and all data that will be given to write must
     * be flushed. */
    cdc_cons->console.fs.flags = (FS_BLOCK | FS_FLUSH_WRITE);

    /* Create pipes to manage data for this device. */
    /* These pipes must be accessed with interrupts disabled. */
    pipe_create(&cdc_cons->rx_pipe, cdc_cons->rx_pipe.fs.name, cdc_cons->rx_pipe_buffer, USB_CDC_CONSOLE_RX_PIPE_SIZE);
    pipe_create(&cdc_cons->tx_pipe, cdc_cons->tx_pipe.fs.name, cdc_cons->tx_pipe_buffer, USB_CDC_CONSOLE_TX_PIPE_SIZE);

} /* usb_cdc_console_register */

/*
 * usb_cdc_console_unregister
 * @cdc_cons: CDC console needed to be unregistered.
 * This function will register a USB CDC console.
 */
void usb_cdc_console_unregister(CDC_CONSOLE *cdc_cons)
{
    /* Destroy the pipe. */
    pipe_destroy(&cdc_cons->rx_pipe);
    pipe_destroy(&cdc_cons->tx_pipe);

    /* Unregister this console with the file system. */
    console_unregister(&cdc_cons->console);

} /* usb_cdc_console_unregister */

/*
 * usb_cdc_fun_console_handle_rx
 * @cdc_cons: USB CDC function console context.
 * @buf: Buffer in which data was received.
 * @nbytes: Number of bytes received.
 * This function will be called whenever there is some data available on a CDC
 * console device.
 */
void usb_cdc_fun_console_handle_rx(CDC_CONSOLE *cdc_cons, char *buf, int32_t nbytes)
{
    /* No need to open the pipe, just write on it. */
    pipe_write(&cdc_cons->rx_pipe, buf, nbytes);

    /* Tell upper layer that some data is now available. */
    fd_data_available(&cdc_cons->console);

} /* usb_cdc_fun_console_handle_rx */

/*
 * usb_cdc_fun_console_handle_tx
 * @cdc_cons: USB CDC function console context.
 * @buf: Buffer in which data will be copied.
 * @nbytes: Number of bytes that can be copied.
 * This function will be called whenever an SOF or a TX event is received. This
 * function will see if we need to send some data and copy the required data
 * in the given buffer.
 */
int32_t usb_cdc_fun_console_handle_tx(CDC_CONSOLE *cdc_cons, char *buf, int32_t nbytes)
{
    /* No need to open the pipe, just write on it. */
    nbytes = pipe_read(&cdc_cons->tx_pipe, buf, nbytes);

    /* Tell upper layer that how much space is available. */
    fd_space_available(cdc_cons, cdc_cons->tx_pipe.fs.space_available(&cdc_cons->tx_pipe));

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
 * usb_cdc_fun_space_available.
 * @cdc_cons: USB CDC function console context.
 * This function will return number of bytes that can be written on this console.
 */
static int32_t usb_cdc_fun_space_available(void *fd)
{
    /* Return number of bytes that can be written on TX PIPE. */
    return (((CDC_CONSOLE *)fd)->tx_pipe.fs.space_available(&((CDC_CONSOLE *)fd)->tx_pipe));

} /* usb_cdc_fun_space_available */

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
    uint32_t int_level = GET_INTERRUPT_LEVEL();
    int32_t nbytes;

    /* Do not preempt this task. */
    scheduler_lock();

    /* We don't want any interrupts from USB while we are doing this. */
    DISABLE_INTERRUPTS();

    /* Read data from the PIPE. */
    nbytes = pipe_read(&cdc->rx_pipe, buffer, size);

    /* Check if RX pipe is now flushed. */
    if (!(cdc->rx_pipe.fs.flags & FS_DATA_AVAILABLE))
    {
        /* also flush the CDC console. */
        fd_data_flushed(fd);
    }

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(int_level);

    /* Enable scheduler. */
    scheduler_unlock();

    /* Return number of bytes. */
    return (nbytes);

} /* usb_cdc_fun_console_read */

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
    uint32_t int_level = GET_INTERRUPT_LEVEL();

    /* Do not preempt this task. */
    scheduler_lock();

    /* We don't want any interrupts from USB while we are doing this. */
    DISABLE_INTERRUPTS();

    /* Push data on the TX pipe. */
    size = pipe_write(&cdc->tx_pipe, buffer, size);

    /* Check if there is no more space in the TX pipe. */
    if (cdc->tx_pipe.fs.flags & FS_NO_MORE_SPACE)
    {
        /* Tell the file system to block the write until there is some space
         * is available. */
        fd_space_consumed(fd);
    }

    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(int_level);

    /* Enable scheduler. */
    scheduler_unlock();

    /* Return number of bytes. */
    return (size);

} /* usb_cdc_fun_console_write */

#endif /* USB_CDC_CONSOLE */
