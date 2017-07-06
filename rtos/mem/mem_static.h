/*
 * mem_static.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form, direct or indirect) the author will not be liable for any
 * outcome.
 */

#ifndef MEM_STATIC_H
#define MEM_STATIC_H

#include <mem.h>

/* Type definitions. */
typedef struct _mem_static
{
    /* Required data to maintain a simple static memory region. */
    uint8_t     *current_ptr;
    uint8_t     *end_ptr;

#ifdef MEMGR_STATS
    /* Total memory size.  */
    uint32_t    mem_size;
#endif

} MEM_STATIC;

/* Function prototypes. */
void mem_static_init_region(MEM_STATIC *, uint8_t *, uint8_t *);
uint8_t *mem_static_alloc_region(MEM_STATIC *, uint32_t);
uint8_t *mem_static_dealloc_region(uint8_t *);

#endif /* MEM_STATIC_H */
