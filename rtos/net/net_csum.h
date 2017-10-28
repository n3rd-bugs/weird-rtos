/*
 * net_csum.c
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
#ifndef NET_CSUM_H
#define NET_CSUM_H
#include <kernel.h>

#ifdef CONFIG_NET

/* Checksum helper macros. */
#define NET_CSUM_ADD(a, b)  {                                           \
                                /* Add complement of two sums. */       \
                                a = (~(a) & 0xFFFF) + (~(b) & 0xFFFF);  \
                                                                        \
                                /* Add carry of sum. */                 \
                                a = (a & 0xFFFF) + (a >> 16);           \
                                a = (a & 0xFFFF) + (a >> 16);           \
                                                                        \
                                /* Return complement of result. */      \
                                a = (~(a) & 0xFFFF);                    \
                            }

/* This will calculate 8-bit checksum. */
#define NET_CSUM_BYTE(a, b) ((((uint16_t)(a) + (uint16_t)(b)) & (0xFF)) + ((((uint16_t)(a) + (uint16_t)(b))  >>  8) & (0xFF)))

/* Function prototypes. */
int32_t net_pseudo_csum_calculate(FS_BUFFER *, uint32_t, uint32_t, uint8_t, uint16_t, uint32_t, uint8_t, uint16_t *);
uint16_t net_csum_calculate(FS_BUFFER *, int32_t, uint32_t);

#endif /* CONFIG_NET */
#endif /* NET_CSUM_H */
