/*
 * net_csum.c
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
#ifndef NET_CSUM_H
#define NET_CSUM_H
#include <os.h>

#ifdef CONFIG_NET

/* Function prototypes. */
uint16_t net_csum_calculate(FS_BUFFER *);

#endif /* CONFIG_NET */
#endif /* NET_CSUM_H */
