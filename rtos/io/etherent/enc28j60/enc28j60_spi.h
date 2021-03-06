/*
 * enc28j60_spi.h
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
#ifndef _ENC28J60_SPI_H_
#define _ENC28J60_SPI_H_
#include <kernel.h>
#include <ethernet.h>

#ifdef ETHERNET_ENC28J60

/* ENC28J60 definitions. */
#define ENC28J60_NON_BANKED         (0x1A)
#define ENC28J60_ADDR_MASK          (0x1F)
#define ENC28J60_MAC_MII_MASK       (0x20)
#define ENC28J60_BANK_MASK          (0xC0)
#define ENC28J60_BANK_SHIFT         (6)

/* ENC28J60 operation definitions. */
#define ENC28J60_OP_READ_CTRL       (0x0)
#define ENC28J60_OP_READ_BUFFER     (0x20)
#define ENC28J60_OP_WRITE_CTRL      (0x40)
#define ENC28J60_OP_WRITE_BUFFER    (0x60)
#define ENC28J60_OP_BIT_SET         (0x80)
#define ENC28J60_OP_BIT_CLR         (0xA0)
#define ENC28J60_OP_RESET           (0xE0)

/* ENC28J60 common value definitions. */
#define ENC28J60_VALUE_RESET        (0xFF)

/* ENC28J60 address definitions. */
#define ENC28J60_BANK_0             (0x0)
#define ENC28J60_BANK_1             (0x40)
#define ENC28J60_BANK_2             (0x80)
#define ENC28J60_BANK_3             (0xC0)
#define ENC28J60_ADDR_ERDPTL        (ENC28J60_BANK_0 | 0x0)
#define ENC28J60_ADDR_ERDPTH        (ENC28J60_BANK_0 | 0x1)
#define ENC28J60_ADDR_EWRPTL        (ENC28J60_BANK_0 | 0x2)
#define ENC28J60_ADDR_EWRPTH        (ENC28J60_BANK_0 | 0x3)
#define ENC28J60_ADDR_ETXSTL        (ENC28J60_BANK_0 | 0x4)
#define ENC28J60_ADDR_ETXSTH        (ENC28J60_BANK_0 | 0x5)
#define ENC28J60_ADDR_ETXNDL        (ENC28J60_BANK_0 | 0x6)
#define ENC28J60_ADDR_ETXNDH        (ENC28J60_BANK_0 | 0x7)
#define ENC28J60_ADDR_ERXSTL        (ENC28J60_BANK_0 | 0x8)
#define ENC28J60_ADDR_ERXSTH        (ENC28J60_BANK_0 | 0x9)
#define ENC28J60_ADDR_ERXNDL        (ENC28J60_BANK_0 | 0xA)
#define ENC28J60_ADDR_ERXNDH        (ENC28J60_BANK_0 | 0xB)
#define ENC28J60_ADDR_ERXRDPTL      (ENC28J60_BANK_0 | 0xC)
#define ENC28J60_ADDR_ERXRDPTH      (ENC28J60_BANK_0 | 0xD)
#define ENC28J60_ADDR_ERXFCON       (ENC28J60_BANK_1 | 0x18)
#define ENC28J60_ADDR_EPKTCNT       (ENC28J60_BANK_1 | 0x19)
#define ENC28J60_ADDR_MACON1        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x0)
#define ENC28J60_ADDR_MACON3        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x2)
#define ENC28J60_ADDR_MABBIPG       (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x4)
#define ENC28J60_ADDR_MAIPGL        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x6)
#define ENC28J60_ADDR_MAIPGH        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x7)
#define ENC28J60_ADDR_MAMXFLL       (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0xA)
#define ENC28J60_ADDR_MAMXFLH       (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0xB)
#define ENC28J60_ADDR_MICMD         (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x12)
#define ENC28J60_ADDR_MIREGADR      (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x14)
#define ENC28J60_ADDR_MIWRL         (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x16)
#define ENC28J60_ADDR_MIWRH         (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x17)
#define ENC28J60_ADDR_MIRDL         (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x18)
#define ENC28J60_ADDR_MIRDH         (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_2 | 0x19)
#define ENC28J60_ADDR_MAADR1        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0x0)
#define ENC28J60_ADDR_MAADR0        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0x1)
#define ENC28J60_ADDR_MAADR3        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0x2)
#define ENC28J60_ADDR_MAADR2        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0x3)
#define ENC28J60_ADDR_MAADR5        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0x4)
#define ENC28J60_ADDR_MAADR4        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0x5)
#define ENC28J60_ADDR_MISTAT        (ENC28J60_MAC_MII_MASK | ENC28J60_BANK_3 | 0xA)
#define ENC28J60_ADDR_EREVID        (ENC28J60_BANK_3 | 0x12)

/* ENC28J60 non-banked control registers. */
#define ENC28J60_ADDR_EIE           (0x1B)
#define ENC28J60_ADDR_EIR           (0x1C)
#define ENC28J60_ADDR_ESTAT         (0x1D)
#define ENC28J60_ADDR_ECON2         (0x1E)
#define ENC28J60_ADDR_ECON1         (0x1F)

/* ENC28J60 opcode address definitions. */
#define ENC28J60_ADDR_RESET         (0x1F)
#define ENC28J60_ADDR_BUFFER        (0x1A)

/* ENC28J60 PHY registers. */
#define ENC28J60_ADDR_PHCON1        (0x0)
#define ENC28J60_ADDR_PHSTAT1       (0x1)
#define ENC28J60_ADDR_PHCON2        (0x10)
#define ENC28J60_ADDR_PHSTAT2       (0x11)
#define ENC28J60_ADDR_PHIE          (0x12)
#define ENC28J60_ADDR_PHIR          (0x13)

/* ENC28J60 ERXFCON register definitions. */
#define ENC28J60_ERXFCON_UCEN       (0x80)
#define ENC28J60_ERXFCON_ANDOR      (0x40)
#define ENC28J60_ERXFCON_CRCEN      (0x20)
#define ENC28J60_ERXFCON_PMEN       (0x10)
#define ENC28J60_ERXFCON_MPEN       (0x8)
#define ENC28J60_ERXFCON_HTEN       (0x4)
#define ENC28J60_ERXFCON_MCEN       (0x2)
#define ENC28J60_ERXFCON_BCEN       (0x1)

/* ENC28J60 MACON1 register definitions */
#define ENC28J60_MACON1_LOOPBK      (0x10)
#define ENC28J60_MACON1_TXPAUS      (0x8)
#define ENC28J60_MACON1_RXPAUS      (0x4)
#define ENC28J60_MACON1_PASSALL     (0x2)
#define ENC28J60_MACON1_MARXEN      (0x1)

/* ENC28J60 MACON3 register definitions. */
#define ENC28J60_MACON3_PADCFG2     (0x80)
#define ENC28J60_MACON3_PADCFG1     (0x40)
#define ENC28J60_MACON3_PADCFG0     (0x20)
#define ENC28J60_MACON3_TXCRCEN     (0x10)
#define ENC28J60_MACON3_PHDRLEN     (0x8)
#define ENC28J60_MACON3_HFRMLEN     (0x4)
#define ENC28J60_MACON3_FRMLNEN     (0x2)
#define ENC28J60_MACON3_FULDPX      (0x1)

/* ENC28J60 MICMD register definitions. */
#define ENC28J60_MICMD_MIISCAN      (0x2)
#define ENC28J60_MICMD_MIIRD        (0x1)

/* ENC28J60 MISTAT register definitions. */
#define ENC28J60_MISTAT_NVALID      (0x4)
#define ENC28J60_MISTAT_SCAN        (0x2)
#define ENC28J60_MISTAT_BUSY        (0x1)

/* ENC28J60 EIE register definitions. */
#define ENC28J60_EIE_INTIE          (0x80)
#define ENC28J60_EIE_PKTIE          (0x40)
#define ENC28J60_EIE_DMAIE          (0x20)
#define ENC28J60_EIE_LINKIE         (0x10)
#define ENC28J60_EIE_TXIE           (0x8)
#define ENC28J60_EIE_WOLIE          (0x4)
#define ENC28J60_EIE_TXERIE         (0x2)
#define ENC28J60_EIE_RXERIE         (0x1)

/* ENC28J60 EIR register definitions. */
#define ENC28J60_EIR_PKTIF          (0x40)
#define ENC28J60_EIR_DMAIF          (0x20)
#define ENC28J60_EIR_LINKIF         (0x10)
#define ENC28J60_EIR_TXIF           (0x8)
#define ENC28J60_EIR_WOLIF          (0x4)
#define ENC28J60_EIR_TXERIF         (0x2)
#define ENC28J60_EIR_RXERIF         (0x1)

/* ENC28J60 ESTAT Register Bit Definitions */
#define ENC28J60_ESTAT_INT          (0x80)
#define ENC28J60_ESTAT_LATECOL      (0x10)
#define ENC28J60_ESTAT_RXBUSY       (0x4)
#define ENC28J60_ESTAT_TXABRT       (0x2)
#define ENC28J60_ESTAT_CLKRDY       (0x1)

/* ENC28J60 ECON2 register definitions. */
#define ENC28J60_ECON2_AUTOINC      (0x80)
#define ENC28J60_ECON2_PKTDEC       (0x40)
#define ENC28J60_ECON2_PWRSV        (0x20)
#define ENC28J60_ECON2_VRPS         (0x8)

/* ENC28J60 ECON1 register definitions. */
#define ENC28J60_ECON1_TXRST        (0x80)
#define ENC28J60_ECON1_RXRST        (0x40)
#define ENC28J60_ECON1_DMAST        (0x20)
#define ENC28J60_ECON1_CSUMEN       (0x10)
#define ENC28J60_ECON1_TXRTS        (0x8)
#define ENC28J60_ECON1_RXEN         (0x4)
#define ENC28J60_ECON1_BSEL1        (0x2)
#define ENC28J60_ECON1_BSEL0        (0x1)

/* ENC28J60 PHCON1 register definitions. */
#define ENC28J60_PHCON1_PRST        (0x8000)
#define ENC28J60_PHCON1_PLOOPBK     (0x4000)
#define ENC28J60_PHCON1_PPWRSV      (0x800)
#define ENC28J60_PHCON1_PDPXMD      (0x100)

/* ENC28J60 PHCON2 register definitions. */
#define ENC28J60_PHCON2_FRCLINK     (0x4000)
#define ENC28J60_PHCON2_TXDIS       (0x2000)
#define ENC28J60_PHCON2_JABBER      (0x400)
#define ENC28J60_PHCON2_HDLDIS      (0x100)

/* ENC28J60 PHIE register definitions. */
#define ENC28J60_PHIE_PLNKIE        (0x10)
#define ENC28J60_PHIE_PGEIE         (0x2)

/* ENC28J60 PHIR register definitions. */
#define ENC28J60_PHIR_PLNKIF        (0x10)
#define ENC28J60_PHIR_PGEIF         (0x2)

/* ENC28J60 PHSTAT2 register definitions. */
#define ENC28J60_PHSTAT2_TXSTAT     (0x2000)
#define ENC28J60_PHSTAT2_RXSTAT     (0x1000)
#define ENC28J60_PHSTAT2_COLSTAT    (0x800)
#define ENC28J60_PHSTAT2_LSTAT      (0x400)
#define ENC28J60_PHSTAT2_DPXSTAT    (0x200)
#define ENC28J60_PHSTAT2_PLRITY     (0x20)

/* Function prototypes. */
int32_t enc28j60_write_phy(ENC28J60 *, uint8_t, uint16_t);
int32_t enc28j60_read_phy(ENC28J60 *, uint8_t, uint16_t *);
int32_t enc28j60_write_buffer(ENC28J60 *, uint16_t, uint8_t *, int32_t);
int32_t enc28j60_read_buffer(ENC28J60 *, uint16_t, uint8_t *, int32_t);
int32_t enc28j60_write_word(ENC28J60 *, uint8_t, uint16_t);
int32_t enc28j60_read_word(ENC28J60 *, uint8_t, uint16_t *);
int32_t enc28j60_write_read_op(ENC28J60 *, uint8_t, uint8_t, uint8_t, uint8_t *, int32_t);

#endif /* ETHERNET_ENC28J60 */
#endif /* _ENC28J60_SPI_H_ */
