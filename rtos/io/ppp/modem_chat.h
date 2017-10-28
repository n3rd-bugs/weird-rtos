/*
 * modem_chat.h
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
#ifndef _MODEM_CHAT_H_
#define _MODEM_CHAT_H_

#include <kernel.h>

#ifdef PPP_MODEM_CHAT

/* Status code definitions. */
#define MODEM_CHAT_IGNORE       -930

/* Function prototypes. */
int32_t modem_chat_process(FD, FS_BUFFER *);

#endif /* PPP_MODEM_CHAT */

#endif /* _MODEM_CHAT_H_ */
