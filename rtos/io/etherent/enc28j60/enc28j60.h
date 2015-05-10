/*
 * enc28j60.h
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
#ifndef _ENC28J60_H_
#define _ENC28J60_H_
#include <os.h>

#ifdef CONFIG_ENC28J60
#ifndef CONFIG_NET
#error "Networking stack required by ENC28j60."
#endif
#ifndef CONFIG_SPI
#error "SPI required by ENC28j60."
#endif
#include <spi.h>
#include <condition.h>

/* ENC28J60 device flags. */
#define ENC28J60_FLAG_INIT  0x01

/* ENC28j60 device structure. */
typedef struct _enc28j60_device
{
    /* SPI device structure. */
    SPI_DEVICE  spi;

    /* Networking condition for this device. */
    CONDITION   condition;

    /* Suspend structure to be used. */
    SUSPEND     suspend;

    /* Current selected memory block. */
    uint8_t     mem_block;

    /* Device flags for enc28j60 device. */
    uint8_t     flags;

    /* Padding variable. */
    uint8_t     pad[2];

} ENC28J60;

/* Function prototypes. */
void enc28j60_init(ENC28J60 *);

#endif /* CONFIG_ENC28J60 */
#endif /* _ENC28J60_H_ */
