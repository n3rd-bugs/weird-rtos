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
#define ENC28J60_OP_READ_CTRL       (0x00)
#define ENC28J60_OP_READ_BUFFER     (0x20)
#define ENC28J60_OP_WRITE_CTRL      (0x40)
#define ENC28J60_OP_WRITE_BUFFER    (0x60)
#define ENC28J60_OP_BIT_SET         (0x80)
#define ENC28J60_OP_BIT_CLR         (0xA0)
#define ENC28J60_OP_RESET           (0xE0)

/* ENC28J60 address definitions. */
#define ENC28J60_BANK_0             (ENC28J60_BANKED_MASK | 0x00)
#define ENC28J60_BANK_1             (ENC28J60_BANKED_MASK | 0x40)
#define ENC28J60_BANK_2             (ENC28J60_BANKED_MASK | 0x80)
#define ENC28J60_BANK_3             (ENC28J60_BANKED_MASK | 0xC0)
#define ENC28J60_ADDR_ERXFCON       (ENC28J60_BANK_1 | 0x18)
#define ENC28J60_ADDR_MACON3        (ENC28J60_BANK_2 | 0x02)
#define ENC28J60_ADDR_MABBIPG       (ENC28J60_BANK_2 | 0x04)
#define ENC28J60_ADDR_MAIPGL        (ENC28J60_BANK_2 | 0x06)
#define ENC28J60_ADDR_MAMXFLL       (ENC28J60_BANK_2 | 0x0A)
#define ENC28J60_ADDR_MAMXFLH       (ENC28J60_BANK_2 | 0x0B)
#define ENC28J60_ADDR_MIREGADR      (ENC28J60_BANK_2 | 0x14)
#define ENC28J60_ADDR_MIWRL         (ENC28J60_BANK_2 | 0x16)
#define ENC28J60_ADDR_MIWRH         (ENC28J60_BANK_2 | 0x17)
#define ENC28J60_ADDR_MAIPGH        (ENC28J60_BANK_2 | 0x07)
#define ENC28J60_ADDR_MISTAT        (ENC28J60_BANK_3 | 0x0A)
#define ENC28J60_ADDR_EREVID        (ENC28J60_BANK_3 | 0x12)

/* ENC28J60 PHY registers. */
#define ENC28J60_ADDR_PHCON1        (0x00)
#define ENC28J60_ADDR_PHSTAT1       (0x01)
#define ENC28J60_ADDR_PHCON2        (0x10)

/* ENC28J60 control registers. */
#define ENC28J60_ADDR_ECON2         (0x1E)
#define ENC28J60_ADDR_ECON1         (0x1F)
#define ENC28J60_ADDR_RESET         (0x1F)

/* ENC28J60 ECON2 register definitions. */
#define ENC28J60_ECON2_AUTOINC      (0x80)
#define ENC28J60_ECON2_PKTDEC       (0x40)
#define ENC28J60_ECON2_PWRSV        (0x20)
#define ENC28J60_ECON2_VRPS         (0x08)

/* ENC28J60 ERXFCON register definitions. */
#define ENC28J60_ERXFCON_UCEN       (0x80)
#define ENC28J60_ERXFCON_ANDOR      (0x40)
#define ENC28J60_ERXFCON_CRCEN      (0x20)
#define ENC28J60_ERXFCON_PMEN       (0x10)
#define ENC28J60_ERXFCON_MPEN       (0x08)
#define ENC28J60_ERXFCON_HTEN       (0x04)
#define ENC28J60_ERXFCON_MCEN       (0x02)
#define ENC28J60_ERXFCON_BCEN       (0x01)

/* ENC28J60 MACON3 register definitions. */
#define ENC28J60_MACON3_PADCFG2     (0x80)
#define ENC28J60_MACON3_PADCFG1     (0x40)
#define ENC28J60_MACON3_PADCFG0     (0x20)
#define ENC28J60_MACON3_TXCRCEN     (0x10)
#define ENC28J60_MACON3_PHDRLEN     (0x08)
#define ENC28J60_MACON3_HFRMLEN     (0x04)
#define ENC28J60_MACON3_FRMLNEN     (0x02)
#define ENC28J60_MACON3_FULDPX      (0x01)

/* ENC28J60 MISTAT register definitions. */
#define ENC28J60_MISTAT_NVALID      (0x04)
#define ENC28J60_MISTAT_SCAN        (0x02)
#define ENC28J60_MISTAT_BUSY        (0x01)

/* ENC28J60 ECON register definitions. */
#define ENC28J60_ECON1_TXRST        (0x80)
#define ENC28J60_ECON1_RXRST        (0x40)
#define ENC28J60_ECON1_DMAST        (0x20)
#define ENC28J60_ECON1_CSUMEN       (0x10)
#define ENC28J60_ECON1_TXRTS        (0x08)
#define ENC28J60_ECON1_RXEN         (0x04)
#define ENC28J60_ECON1_BSEL1        (0x02)
#define ENC28J60_ECON1_BSEL0        (0x01)

/* ENC28J60 PHCON1 register definitions. */
#define ENC28J60_PHCON1_PRST        (0x8000)
#define ENC28J60_PHCON1_PLOOPBK     (0x4000)
#define ENC28J60_PHCON1_PPWRSV      (0x0800)
#define ENC28J60_PHCON1_PDPXMD      (0x0100)

/* ENC28J60 common value definitions. */
#define ENC28J60_VALUE_RESET        (0xFF)

/* ENC28J60 definitions. */
#define ENC28J60_ADDR_MASK          (0x1F)
#define ENC28J60_BANKED_MASK        (0x20)
#define ENC28J60_BANK_MASK          (0xC0)
#define ENC28J60_BANK_SHIFT         (6)

/* Function prototypes. */
int32_t enc28j60_write_phy(ENC28J60 *, uint8_t, uint16_t);
int32_t enc28j60_write_read_op(ENC28J60 *, uint8_t, uint8_t, uint8_t, uint8_t *);

#endif /* CONFIG_ENC28J60 */
#endif /* _ENC28J60_SPI_H_ */
