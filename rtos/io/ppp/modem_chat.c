/*
 * modem_chat.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */
#include <kernel.h>

#ifdef CONFIG_PPP
#include <ppp.h>

#ifdef PPP_MODEM_CHAT
#include <string.h>
#include <fs.h>

/*
 * modem_chat_process
 * @fd: File descriptor on which data was received and reply is needed to be
 *  sent.
 * @rx_buffer: Buffer in which data was received.
 * @return: Returns SUCCESS if data was successfully processed.
 *  MODEM_CHAT_INCOMPLETE is returned if we are in process of receiving the
 *      complete message,
 *  MODEM_CHAT_INVALID will be returned if received data is not recognized and
 *      should be dropped.
 * This function will process modem initialization.
 */
int32_t modem_chat_process(FD fd, FS_BUFFER_LIST *rx_buffer)
{
    FS_BUFFER_LIST *tx_buffer;
    int32_t status = SUCCESS;

    /* For now we will just wait for the "CLIENT" string from the other end. */
    if (((rx_buffer)->total_length == (sizeof("CLIENT") - 1)) &&
        (memcmp((rx_buffer)->list.head->buffer, "CLIENT", sizeof("CLIENT") - 1) == 0))
    {
        /* Get a free buffer so that it can be populate with response. */
        tx_buffer = fs_buffer_get(fd, FS_LIST_FREE, (FS_BUFFER_ACTIVE));

        if (tx_buffer)
        {
            /* Initialize response. */
            ASSERT(fs_buffer_list_push(tx_buffer, "CLIENTSERVER", (sizeof("CLIENTSERVER") - 1), 0) != SUCCESS);

            /* Release lock for file descriptor. */
            fd_release_lock(fd);

            /* Add a transmit buffer. */
            fs_write(fd, (uint8_t *)tx_buffer, sizeof(tx_buffer));

            /* Acquire file descriptor lock. */
            ASSERT(fd_get_lock(fd) != SUCCESS);
        }
    }

    /* If we have received the anticipated modem chat. */
    else if (memcmp((rx_buffer)->list.head->buffer, "CLIENT", (rx_buffer)->total_length) == 0)
    {
        /* Just ignore this for now we will process it when we will have a
         * complete message. */
        status = MODEM_CHAT_INCOMPLETE;
    }

    else
    {
        /* We have not the anticipated data, drop this frame. */
        status = MODEM_CHAT_INVALID;
    }

    /* Return status to the caller. */
    return (status);

} /* modem_chat_process */

#endif /* PPP_MODEM_CHAT */
#endif /* CONFIG_PPP */
