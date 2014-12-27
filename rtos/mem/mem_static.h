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
 * (in any form) the author will not be liable for any legal charges.
 */

#ifndef MEM_STATIC_H
#define MEM_STATIC_H

#include <mem.h>

/* Type definitions. */
typedef struct _mem_static
{
    /* Required data to maintain a simple static memory pool. */
    char        *current_ptr;
    char        *mem_end_ptr;

#ifdef CONFIG_MEMGR_STATS
    /* Total memory size.  */
    uint32_t    mem_size;
#endif

} MEM_STATIC;

/* Function prototypes. */
void mem_static_init_region(MEM_STATIC *, char *, char *);
char *mem_static_alloc_region(MEM_STATIC *mem_static, uint32_t size);
char *mem_static_dealloc_region(MEM_STATIC *mem_static, char *mem_ptr);

#endif /* MEM_STATIC_H */
