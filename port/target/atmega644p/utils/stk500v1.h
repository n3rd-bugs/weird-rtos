/*
 * stk500v1.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#ifndef _STK500V1_H_
#define _STK500V1_H_

/* STK500 version information. */
#define STK_HWVER           1
#define STK_SWMAJ           2   /* Must be greater than 1. */
#define STK_SWMIN           0

/* STK500 command definitions. */
#define STK_OK              0x10
#define STK_FAILED          0x11
#define STK_UNKNOWN         0x12
#define STK_INSYNC          0x14
#define STK_NOSYNC          0x15
#define STK_CRC_EOP         0x20
#define STK_GET_SYNC        0x30
#define STK_GET_SIGN_ON     0x31
#define STK_SET_PARAMETER   0x40
#define STK_GET_PARAMETER   0x41
#define STK_SET_DEVICE      0x42
#define STK_SET_DEVICE_EXT  0x45
#define STK_ENTER_PROGMODE  0x50
#define STK_LEAVE_PROGMODE  0x51
#define STK_CHIP_ERASE      0x52
#define STK_CHECK_AUTOINC   0x53
#define STK_LOAD_ADDRESS    0x55
#define STK_UNIVERSAL       0x56
#define STK_PROG_FLASH      0x60
#define STK_PROG_DATA       0x61
#define STK_PROG_FUSE       0x62
#define STK_PROG_LOCK       0x63
#define STK_PROG_PAGE       0x64
#define STK_PROG_FUSE_EXT   0x65
#define STK_READ_FLASH      0x70
#define STK_READ_DATA       0x71
#define STK_READ_FUSE       0x72
#define STK_READ_LOCK       0x73
#define STK_READ_PAGE       0x74
#define STK_READ_SIGN       0x75
#define STK_READ_OSCCAL     0x76
#define STK_READ_FUSE_EXT   0x77
#define STK_READ_OSCCAL_EXT 0x78
#define STK_READ_HWVER      0x80
#define STK_READ_SWMAJ      0x81
#define STK_READ_SWMIN      0x82
#define STK_READ_PGTYPE     0x93

#endif /* _STK500V1_H_ */
