/*
 * header.h
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
#ifndef _HEADER_H_
#define _HEADER_H_

#include <kernel.h>

/* Header type flags. */
#define HEADER_BIT          0x1
#define HEADER_PROCESS      0x2
#define HEADER_END          0x8000

/* This function will be used to pull/push data from/to the provided buffer. */
typedef int32_t HRD_PULL(void *, uint8_t *, uint32_t, uint16_t);
typedef int32_t HRD_PUSH(void *, uint8_t *, uint32_t, uint16_t);

/* This function will be used to process a header. */
typedef int32_t HRD_PROCESS (void *, uint8_t *, uint32_t);

/* A single header definition. */
typedef struct _header
{
    void        *value;
    uint16_t    size;
    uint16_t    flags;
} HEADER;

/* Header generator machine data. */
typedef struct _hdr_gen_machine
{
    HRD_PUSH    *push;          /* Function that will be called to push data on the buffer. */
} HDR_GEN_MACHINE;

/* Header parse machine data. */
typedef struct _hdr_parse_machine
{
    HRD_PULL    *pull;          /* Function that will be called to pull the data from the buffer. */
} HDR_PARSE_MACHINE;

/* Header process machine data. */
typedef struct _hdr_process_machine
{
    HRD_PULL    *pull;          /* Function that will be called to pull the data from the buffer. */
    uint16_t    header;         /* Current header we are parsing. */
    uint8_t     num_bits;       /* Number of valid bits left. */
    uint8_t     byte;           /* Remaining data in the byte. */
} HDR_PROCESS_MACHINE;

/* Function prototypes. */
void header_gen_machine_init(HDR_GEN_MACHINE *, HRD_PUSH *);
int32_t header_generate(HDR_GEN_MACHINE *, HEADER *, uint32_t, void *);
void header_parse_machine_init(HDR_PARSE_MACHINE *, HRD_PULL *);
int32_t header_parse(HDR_PARSE_MACHINE *, HEADER *, uint32_t, void *);
void header_process_machine_init(HDR_PROCESS_MACHINE *, HRD_PULL *);
int32_t header_process_machine_run(HDR_PROCESS_MACHINE *, void *, const HEADER *, void *, uint8_t *);

#endif /* _HEADER_H_ */
