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
int32_t net_pseudo_csum_calculate(FS_BUFFER *, uint32_t, uint32_t, uint8_t, uint32_t, uint32_t, uint16_t, uint16_t *);
uint16_t net_csum_calculate(FS_BUFFER *, int32_t, uint32_t);

#endif /* CONFIG_NET */
#endif /* NET_CSUM_H */
