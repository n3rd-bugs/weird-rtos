/*
 * net_process.h
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
#ifndef _NET_PROCESS_H_
#define _NET_PROCESS_H_
#include <kernel.h>

#ifdef CONFIG_NET
#include <fs.h>

/* Function prototypes. */
int32_t net_buffer_process(FS_BUFFER *);

#endif /* CONFIG_NET */
#endif /* _NET_PROCESS_H_ */
