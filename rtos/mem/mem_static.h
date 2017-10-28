/*
 * mem_static.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
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
