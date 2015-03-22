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
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>
#include <string.h>

#ifdef PPP_MODEM_CHAT
/*
 * modem_chat_process
 * @fd: File descriptor on which data was received and reply is needed to be
 *  sent.
 * @rx_buffer: Buffer in which data was received.
 * @return: Returns SUCCESS if data was successfully processed.
 *  MODEM_CHAT_IGNORE is returned if data was not recognized.
 * This function will process modem initialization.
 */
int32_t modem_chat_process(FD fd, FS_BUFFER_ONE *rx_buffer)
{
    FS_BUFFER_ONE *tx_buffer;
    int32_t status = SUCCESS;

    /* For now we will just wait for the "CLIENT" string from the other end. */
    if (((rx_buffer)->length == (sizeof("CLIENT") - 1)) &&
        (memcmp((rx_buffer)->buffer, "CLIENT", sizeof("CLIENT") - 1) == 0))
    {
        /* Get a free buffer so that it can be populate with response. */
        tx_buffer = fs_buffer_get(fd, FS_BUFFER_FREE, FS_BUFFER_ACTIVE);

        if (tx_buffer)
        {
            /* Initialize response. */
            OS_ASSERT(fs_buffer_one_push(tx_buffer, "CLIENTSERVER", (sizeof("CLIENTSERVER") - 1), 0) != SUCCESS);

            /* Add a transmit buffer. */
            fs_buffer_add(fd, tx_buffer, FS_BUFFER_TX, FS_BUFFER_ACTIVE);
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
