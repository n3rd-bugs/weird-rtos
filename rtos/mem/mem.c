/*
 * mem.c
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
#include <kernel.h>

#ifdef CONFIG_MEMGR
#include <mem.h>

#ifdef MEMGR_STATIC
MEM_STATIC mem_static_pool;
#endif

#ifdef MEMGR_DYNAMIC
MEM_DYNAMIC mem_dynamic_pool;

/* Total number of pages in dynamic memory. */
#define NUM_PAGES       10
#define MEM_FLAGS       0//MEM_STRICT_ALLOC

/* Dynamic memory allocation configuration. */
MEM_DYN_CFG mem_dyn_cfg [NUM_PAGES] =
{
        { 0x000000FE, 0x00000000, MEM_PAGE_ASC },
        { 0x000000FE, 0x00000000, MEM_PAGE_ASC },
        { 0x00000100, 0x00000000, MEM_PAGE_DEC },
        { 0x000001FF, 0x00000000, 0 },
        { 0x000001FF, 0x00000000, MEM_PAGE_ASC },
        { 0x00000200, 0x00000000, MEM_PAGE_DEC },
        { 0x00000200, 0x00000000, MEM_PAGE_DEC },
        { 0x000002FF, 0x00000000, MEM_PAGE_ASC },
        { 0x00000300, 0x00000000, MEM_PAGE_ASC },
        { 0x00000400, 0x00000820, 0 },
};
#endif

/*
 * mem_init
 * This function initializes memory managers.
 */
void mem_init()
{

#ifdef MEMGR_STATIC
    /* Initialize global static memory region. */
    mem_static_init_region(&mem_static_pool, STATIC_MEM_START, STATIC_MEM_END);
#endif

#ifdef MEMGR_DYNAMIC
    /* Initialize global static memory region. */
    mem_dynamic_init_region(&mem_dynamic_pool, DYNAMIC_MEM_START, DYNAMIC_MEM_END, NUM_PAGES, mem_dyn_cfg, MEM_FLAGS);
#endif

} /* mem_init */

#endif /* CONFIG_MEMGR */
