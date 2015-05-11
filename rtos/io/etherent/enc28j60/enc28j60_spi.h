/*
 * enc28j60_spi.h
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
#ifndef _ENC28J60_SPI_H_
#define _ENC28J60_SPI_H_
#include <os.h>

#ifdef CONFIG_ENC28J60

/* ENC28J60 operation definitions. */
#define ENC28J60_OP_READ_CNTRL      0x00
#define ENC28J60_OP_READ_BUFFER     0x20
#define ENC28J60_OP_WRITE_CNTRL     0x40
#define ENC28J60_OP_WRITE_BUFFER    0x60
#define ENC28J60_OP_BIT_SET         0x80
#define ENC28J60_OP_BIT_CLR         0xA0
#define ENC28J60_OP_RESET           0xE0

/* ENC28J60 address definitions. */
#define ENC28J60_BANK_0             (ENC28J60_BANKED_MASK | 0x00)
#define ENC28J60_BANK_1             (ENC28J60_BANKED_MASK | 0x40)
#define ENC28J60_BANK_2             (ENC28J60_BANKED_MASK | 0x80)
#define ENC28J60_BANK_3             (ENC28J60_BANKED_MASK | 0xC0)
#define ENC28J60_ADDR_EREVID        (ENC28J60_BANK_3 | 0x12)

/* ENC28J60 non banked registers. */
#define ENC28J60_ADDR_ECON          (0x1F)
#define ENC28J60_ADDR_RESET         (0x1F)

/* ENC28J60 common value definitions. */
#define ENC28J60_VALUE_RESET        0xFF

/* ENC28J60 definitions. */
#define ENC28J60_ADDR_MASK          0x1F
#define ENC28J60_BANKED_MASK        0x20
#define ENC28J60_BANK_MASK          0xC0
#define ENC28J60_BANK_SHIFT         (6)

/* ENC28J60 ECON register definitions. */
#define ENC28J60_ECON1_TXRST        0x80
#define ENC28J60_ECON1_RXRST        0x40
#define ENC28J60_ECON1_DMAST        0x20
#define ENC28J60_ECON1_CSUMEN       0x10
#define ENC28J60_ECON1_TXRTS        0x08
#define ENC28J60_ECON1_RXEN         0x04
#define ENC28J60_ECON1_BSEL1        0x02
#define ENC28J60_ECON1_BSEL0        0x01

/* Function prototypes. */
int32_t enc28j60_write_read_op(ENC28J60 *, uint8_t, uint8_t, uint8_t, uint8_t *);

#endif /* CONFIG_ENC28J60 */
#endif /* _ENC28J60_SPI_H_ */
