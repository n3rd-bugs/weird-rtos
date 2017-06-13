/*
 * modem_chat.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <os.h>

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
 *  MODEM_CHAT_IGNORE is returned if data was not recognized.
 * This function will process modem initialization.
 */
int32_t modem_chat_process(FD fd, FS_BUFFER *rx_buffer)
{
    FS_BUFFER *tx_buffer;
    int32_t status = SUCCESS;

    /* For now we will just wait for the "CLIENT" string from the other end. */
    if (((rx_buffer)->total_length == (sizeof("CLIENT") - 1)) &&
        (memcmp((rx_buffer)->list.head->data, "CLIENT", sizeof("CLIENT") - 1) == 0))
    {
        /* Get a free buffer so that it can be populate with response. */
        tx_buffer = fs_buffer_get(fd, FS_BUFFER_LIST, (FS_BUFFER_ACTIVE));

        if (tx_buffer)
        {
            /* Initialize response. */
            OS_ASSERT(fs_buffer_push(tx_buffer, "CLIENTSERVER", (sizeof("CLIENTSERVER") - 1), 0) != SUCCESS);

            /* Release lock for file descriptor. */
            fd_release_lock(fd);

            /* Add a transmit buffer. */
            fs_write(fd, (uint8_t *)tx_buffer, sizeof(tx_buffer));

            /* Acquire file descriptor lock. */
            OS_ASSERT(fd_get_lock(fd) != SUCCESS);
        }
    }
    else
    {
        /* Just ignore this packet. */
        status = MODEM_CHAT_IGNORE;
    }

    /* Return status to the caller. */
    return (status);

} /* modem_chat_process */

#endif /* PPP_MODEM_CHAT */
#endif /* CONFIG_PPP */
