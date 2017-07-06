/*
 * net_process.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
#ifndef _NET_PROCESS_H_
#define _NET_PROCESS_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <fs.h>

/* Function prototypes. */
int32_t net_buffer_process(FS_BUFFER *);

#endif /* CONFIG_NET */
#endif /* _NET_PROCESS_H_ */
