/*
 * modem_chat.h
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
#ifndef _MODEM_CHAT_H_
#define _MODEM_CHAT_H_

#include <os.h>

#ifdef PPP_MODEM_CHAT

/* Status code definitions. */
#define MODEM_CHAT_IGNORE       -2001

/* Function prototypes. */
int32_t modem_chat_process(FD, FS_BUFFER *);

#endif /* PPP_MODEM_CHAT */

#endif /* _MODEM_CHAT_H_ */
