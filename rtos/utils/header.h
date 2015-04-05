/*
 * header.h
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
#ifndef _HEADER_H_
#define _HEADER_H_

#include <os.h>
#include <fs.h>

/* Header type flags. */
#define HEADER_BIT          0x0001
#define HEADER_PROCESS      0x0002
#define HEADER_END          0x8000

/* This function will be used to pull data from the provided buffer. */
typedef int32_t HRD_PULL (void *, uint8_t *, uint32_t);

/* This function will be used to process a header. */
typedef int32_t HRD_PROCESS (void *, uint8_t *, uint32_t);

/* A single header definition. */
typedef struct _header
{
    HRD_PROCESS *process;
    uint16_t    size;
    uint16_t    flags;
} HEADER;

/* Header parse machine data. */
typedef struct _hdr_machine
{
    HRD_PULL    *pull;          /* Function that will be called to pull the data from the buffer. */
    uint16_t    header;         /* Current header we are parsing. */
    uint8_t     num_bits;       /* Number of valid bits left. */
    uint8_t     byte;           /* Remaining data in the byte. */
} HDR_MACHINE;

/* Function prototypes. */
void header_machine_init(HDR_MACHINE *, HRD_PULL *);
int32_t header_machine_run(HDR_MACHINE *, void *, const HEADER *, void *, uint8_t *);

#endif /* _HEADER_H_ */
