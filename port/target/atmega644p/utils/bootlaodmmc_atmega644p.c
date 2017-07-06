/*
 * bootloadmmc_atmega644p.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */
/*------------------------------------------------------------------------/
/  Bitbanging MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/--------------------------------------------------------------------------/
 Features and Limitations:

 * Very Easy to Port
   It uses only 4 bit of GPIO port. No interrupt, no SPI port is used.

 * Platform Independent
   You need to modify only a few macros to control GPIO ports.

 * Low Speed
   The data transfer rate will be several times slower than hardware SPI.

/-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#include <avr/io.h>             /* Include device specific declareation file here */
#include <util/delay.h>
#include <bootload_atmega644p.h>

#ifdef CONFIG_BOOTLOAD
#ifndef BOOTLOADER_LOADED
#ifdef BOOTLOAD_MMC

/* Status code definitions. */
#define STA_NOINIT      0x01    /* Drive not initialized */
#define STA_NODISK      0x02    /* No medium in the drive */
#define STA_PROTECT     0x04    /* Write protected */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC          0x01    /* MMC ver 3 */
#define CT_SD1          0x02    /* SD ver 1 */
#define CT_SD2          0x04    /* SD ver 2 */
#define CT_SDC          (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK        0x08    /* Block addressing */

#define INIT_PORT()     init_port()                     /* Initialize MMC control port (CS=H, CLK=L, DI=H, DO=pu) */
#define DLY_US(n)       _delay_us(n)                    /* Delay n microseconds */

#define CS_H()          (PORTA |= (1 << 3))             /* Set MMC_CS "high" */
#define CS_L()          (PORTA &= (uint8_t)~(1 << 3))   /* Set MMC_CS "low" */
#define CK_H()          (PORTA |= (1 << 2))             /* Set MMC_SCLK "high" */
#define CK_L()          (PORTA &= (uint8_t)~(1 << 2))   /* Set MMC_SCLK "low" */
#define DI_H()          (PORTA |= (1 << 5))             /* Set MMC_DI "high" */
#define DI_L()          (PORTA &= (uint8_t)~(1 << 5))   /* Set MMC_DI "low" */
#define DO              (PINA & (1 << 0))               /* Test for MMC_DO ('H':true, 'L':false) */

/* Results of Disk Functions */
enum {
    RES_OK = 0,     /* 0: Successful */
    RES_ERROR,      /* 1: R/W Error */
    RES_WRPRT,      /* 2: Write Protected */
    RES_NOTRDY,     /* 3: Not Ready */
    RES_PARERR      /* 4: Invalid Parameter */
};

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0            (0)         /* GO_IDLE_STATE */
#define CMD1            (1)         /* SEND_OP_COND */
#define ACMD41          (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8            (8)         /* SEND_IF_COND */
#define CMD9            (9)         /* SEND_CSD */
#define CMD10           (10)        /* SEND_CID */
#define CMD12           (12)        /* STOP_TRANSMISSION */
#define CMD13           (13)        /* SEND_STATUS */
#define ACMD13          (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16           (16)        /* SET_BLOCKLEN */
#define CMD17           (17)        /* READ_SINGLE_BLOCK */
#define CMD18           (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23           (23)        /* SET_BLOCK_COUNT */
#define ACMD23          (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24           (24)        /* WRITE_BLOCK */
#define CMD25           (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32           (32)        /* ERASE_ER_BLK_START */
#define CMD33           (33)        /* ERASE_ER_BLK_END */
#define CMD38           (38)        /* ERASE */
#define CMD55           (55)        /* APP_CMD */
#define CMD58           (58)        /* READ_OCR */

static void init_port (void) BOOTLOAD_SECTION;
static void xmit_mmc (const uint8_t*,uint32_t) BOOTLOAD_SECTION;
static void rcvr_mmc (uint8_t *, uint32_t) BOOTLOAD_SECTION;
static int wait_ready (void) BOOTLOAD_SECTION;
static void deselect (void) BOOTLOAD_SECTION;
static int select (void) BOOTLOAD_SECTION;
static int rcvr_datablock (uint8_t *, uint32_t, uint8_t, uint8_t) BOOTLOAD_SECTION;
#if _USE_WRITE
static int xmit_datablock (const uint8_t *, uint8_t) BOOTLOAD_SECTION;
#endif
static uint8_t send_cmd(uint8_t, uint32_t) BOOTLOAD_SECTION;

static
void init_port (void)
{
    /* Initialize the GPIO port MMC attached (CS=High, SCLK=Low, DI=High, DO=In/pullup) */
    DDRA |= ((1 << 2) | (1 << 3) | (1 << 5));
    DDRA &= ((uint8_t)~(1 << 0));
    CS_H();
    CK_L();
    DI_H();
}

/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card (bitbanging)                               */
/*-----------------------------------------------------------------------*/

static
void xmit_mmc (
    const uint8_t* buff,   /* Data to be sent */
    uint32_t bc             /* Number of bytes to send */
)
{
    uint8_t d;

    do {
        d = *buff++;    /* Get a byte to be sent */
        if (d & 0x80) DI_H(); else DI_L();  /* bit7 */
        CK_H(); CK_L();
        if (d & 0x40) DI_H(); else DI_L();  /* bit6 */
        CK_H(); CK_L();
        if (d & 0x20) DI_H(); else DI_L();  /* bit5 */
        CK_H(); CK_L();
        if (d & 0x10) DI_H(); else DI_L();  /* bit4 */
        CK_H(); CK_L();
        if (d & 0x08) DI_H(); else DI_L();  /* bit3 */
        CK_H(); CK_L();
        if (d & 0x04) DI_H(); else DI_L();  /* bit2 */
        CK_H(); CK_L();
        if (d & 0x02) DI_H(); else DI_L();  /* bit1 */
        CK_H(); CK_L();
        if (d & 0x01) DI_H(); else DI_L();  /* bit0 */
        CK_H(); CK_L();
    } while (--bc);
}

/*-----------------------------------------------------------------------*/
/* Receive bytes from the card (bitbanging)                              */
/*-----------------------------------------------------------------------*/

static
void rcvr_mmc (
    uint8_t *buff, /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
)
{
    uint8_t r;

    DI_H(); /* Send 0xFF */

    do {
        r = 0;
        CK_H();
        if (DO) r++;   /* bit7 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit6 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit5 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit4 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit3 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit2 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit1 */
        CK_L(); CK_H();
        r <<= 1; if (DO) r++;   /* bit0 */
        CK_L();
        *buff++ = r;            /* Store a received byte */
    } while (--bc);
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)   /* 1:OK, 0:Timeout */
{
    uint8_t d;
    uint32_t tmr;

    for (tmr = 5000; tmr; tmr--) {  /* Wait for ready in timeout of 500ms */
        rcvr_mmc(&d, 1);
        if (d == 0xFF) break;
        DLY_US(100);
    }

    return tmr ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
    uint8_t d;

    CS_H();             /* Set CS# high */
    rcvr_mmc(&d, 1);    /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

static
int select (void)   /* 1:OK, 0:Timeout */
{
    uint8_t d;

    CS_L();             /*Set CS# low */
    rcvr_mmc(&d, 1);    /* Dummy clock (force DO enabled) */
    if (wait_ready()) return 1; /* Wait for card ready */

    deselect();
    return 0;           /* Failed */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (    /* 1:OK, 0:Failed */
    uint8_t *buff,      /* Data buffer to store received data */
    uint32_t btr,       /* Byte count */
    uint8_t first,      /* If this is the first call */
    uint8_t last        /* If this is the last call */
)
{
    uint8_t d[2];
    uint32_t tmr;

    if (first == TRUE) {
        for (tmr = 1000; tmr; tmr--) {  /* Wait for data packet in timeout of 100ms */
            rcvr_mmc(d, 1);
            if (d[0] != 0xFF) break;
            DLY_US(100);
        }
        if (d[0] != 0xFE) return 0;     /* If not valid data token, return with error */
    }

    if (btr > 0) {
        rcvr_mmc(buff, btr);            /* Receive the data block into buffer */
    }

    if (last == TRUE) {
        rcvr_mmc(d, 2);                 /* Discard CRC */
    }

    return 1;                       /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
static
int xmit_datablock (    /* 1:OK, 0:Failed */
    const uint8_t *buff,   /* 512 byte data block to be transmitted */
    uint8_t token          /* Data/Stop token */
)
{
    uint8_t d[2];

    if (!wait_ready()) return 0;

    d[0] = token;
    xmit_mmc(d, 1);             /* Xmit a token */
    if (token != 0xFD) {        /* Is it data token? */
        xmit_mmc(buff, 512);    /* Xmit the 512 byte data block to MMC */
        rcvr_mmc(d, 2);         /* Xmit dummy CRC (0xFF,0xFF) */
        rcvr_mmc(d, 1);         /* Receive data response */
        if ((d[0] & 0x1F) != 0x05)  /* If not accepted, return with error */
            return 0;
    }

    return 1;
}
#endif /* _USE_WRITE */

/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

static
uint8_t send_cmd (     /* Returns command response (bit7==1:Send failed)*/
    uint8_t cmd,       /* Command byte */
    uint32_t arg       /* Argument */
)
{
    uint8_t n, d, buf[6];

    if (cmd & 0x80) {   /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        n = send_cmd(CMD55, 0);
        if (n > 1) return n;
    }

    /* Select the card and wait for ready except to stop multiple block read */
    if ((cmd != CMD12) || (cmd != CMD0)) {
        deselect();
        if (!select()) return 0xFF;
    }

    /* Send a command packet */
    buf[0] = 0x40 | cmd;            /* Start + Command index */
    buf[1] = (uint8_t)(arg >> 24);     /* Argument[31..24] */
    buf[2] = (uint8_t)(arg >> 16);     /* Argument[23..16] */
    buf[3] = (uint8_t)(arg >> 8);      /* Argument[15..8] */
    buf[4] = (uint8_t)arg;             /* Argument[7..0] */
    n = 0x01;                       /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;      /* (valid CRC for CMD0(0)) */
    if (cmd == CMD8) n = 0x87;      /* (valid CRC for CMD8(0x1AA)) */
    buf[5] = n;
    xmit_mmc(buf, 6);

    /* Receive command response */
    if (cmd == CMD12) rcvr_mmc(&d, 1);  /* Skip a stuff byte when stop reading */
    n = 10;                             /* Wait for a valid response in timeout of 10 attempts */
    do
        rcvr_mmc(&d, 1);
    while ((d & 0x80) && --n);

    return d;           /* Return with the response value */
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

int32_t bootload_disk_initialize (uint8_t *card_type)
{
    uint8_t n, ty, cmd, buf[4];
    uint32_t tmr;
    int32_t s, try = 10;

    INIT_PORT();                /* Initialize control port */
    for (n = 10; n; n--) rcvr_mmc(buf, 1);  /* 80 dummy clocks */

    if (!select()) return STA_NOINIT;

    while (--try) {
        if (send_cmd(CMD0, 0) == 1) {
            break;
        }
        DLY_US(1000);
    }

    ty = 0;
    if (send_cmd(CMD0, 0) == 1) {           /* Put the card SPI mode */
        if (send_cmd(CMD8, 0x1AA) == 1) {   /* Is the card SDv2? */
            rcvr_mmc(buf, 4);                           /* Get trailing return value of R7 resp */
            if (buf[2] == 0x01 && buf[3] == 0xAA) {     /* The card can work at vdd range of 2.7-3.6V */
                for (tmr = 1000; tmr; tmr--) {          /* Wait for leaving idle state (ACMD41 with HCS bit) */
                    if (send_cmd(ACMD41, 1UL << 30) == 0) break;
                    DLY_US(1000);
                }
                if (tmr && send_cmd(CMD58, 0) == 0) {   /* Check CCS bit in the OCR */
                    rcvr_mmc(buf, 4);
                    ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* Check if the card SDv2 */
                }
            }
        } else {                            /* SDv1 or MMCv3 */
            if (send_cmd(ACMD41, 0) <= 1) {
                ty = CT_SD1; cmd = ACMD41;  /* SDv1 */
            } else {
                ty = CT_MMC; cmd = CMD1;    /* MMCv3 */
            }
            for (tmr = 1000; tmr; tmr--) {          /* Wait for the card leaves idle state */
                if (send_cmd(cmd, 0) == 0) break;
                DLY_US(1000);
            }
            if (!tmr || send_cmd(CMD16, 512) != 0)  /* Set R/W block length to 512 */
                ty = 0;
        }
    }
    *card_type = ty;
    s = ty ? 0 : STA_NOINIT;

    deselect();

    return s;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

int32_t bootload_disk_read (
    uint8_t type,           /* Card type flags */
    uint8_t *buff,          /* Pointer to the data buffer to store read data */
    uint32_t sector,        /* Start sector number (LBA) */
    uint32_t count,         /* Number of bytes to read */
    uint32_t *offset         /* Current sector offset */
)
{
    uint32_t this_count;

    if (!(type & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

    if (*offset == 0) {
        if (send_cmd(CMD18, sector)) return RES_ERROR;
        if (!(rcvr_datablock(NULL, 0, TRUE, FALSE))) return RES_ERROR;
    }

    while ((count > 0) && (buff != NULL)) {
        if (count > (512 - (*offset % 512))) {
            this_count = 512 - (*offset % 512);
        } else {
            this_count = count;
        }

        /* Receive required data. */
        if (!(rcvr_datablock(buff, this_count, FALSE, FALSE))) return RES_ERROR;

        *offset += this_count;
        count -= this_count;
        buff += this_count;

        if ((*offset % 512) == 0) {
            if (!(rcvr_datablock(NULL, 0, FALSE, TRUE))) return RES_ERROR;;
            if (!(rcvr_datablock(NULL, 0, TRUE, FALSE))) return RES_ERROR;;
        }
    }

    if (buff == NULL) {
        send_cmd(CMD12, 0);   /* STOP_TRANSMISSION */
        deselect();
    }

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
int32_t bootload_disk_write (
    uint8_t type,           /* Card type flags */
    const uint8_t *buff,   /* Pointer to the data to be written */
    uint32_t sector,       /* Start sector number (LBA) */
    uint32_t count          /* Sector count (1..128) */
)
{
    if (!count) return RES_PARERR;
    if (!(type & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

    if (count == 1) {   /* Single block write */
        if ((send_cmd(CMD24, sector) == 0)  /* WRITE_BLOCK */
            && xmit_datablock(buff, 0xFE))
            count = 0;
    }
    else {              /* Multiple block write */
        if (type & CT_SDC) send_cmd(ACMD23, count);
        if (send_cmd(CMD25, sector) == 0) { /* WRITE_MULTIPLE_BLOCK */
            do {
                if (!xmit_datablock(buff, 0xFC)) break;
                buff += 512;
            } while (--count);
            if (!xmit_datablock(0, 0xFD))   /* STOP_TRAN token */
                count = 1;
        }
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
int32_t bootload_disk_ioctl (
    uint8_t ctrl,      /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    int32_t res;
    uint8_t n, csd[16];
    uint32_t cs;

    res = RES_ERROR;
    switch (ctrl) {
    case CTRL_SYNC :        /* Make sure that no pending write process */
        if (select()) res = RES_OK;
        break;

    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (uint32_t) */
        if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16, TRUE, TRUE)) {
            if ((csd[0] >> 6) == 1) {   /* SDC ver 2.00 */
                cs = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
                *(uint32_t*)buff = cs << 10;
            } else {                    /* SDC ver 1.XX or MMC */
                n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                cs = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
                *(uint32_t*)buff = cs << (n - 9);
            }
            res = RES_OK;
        }
        break;

    case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (uint32_t) */
        *(uint32_t*)buff = 128;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
    }

    deselect();

    return res;
}
#endif
#endif /* BOOTLOAD_MMC */
#endif /* BOOTLOADER_LOADED */
#endif /* CONFIG_BOOTLOAD */
