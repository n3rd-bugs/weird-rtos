/*
 * mem.c
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
        { 0xFE,     0x0,    MEM_PAGE_ASC },
        { 0xFE,     0x0,    MEM_PAGE_ASC },
        { 0x100,    0x0,    MEM_PAGE_DEC },
        { 0x1FF,    0x0,    0 },
        { 0x1FF,    0x0,    MEM_PAGE_ASC },
        { 0x200,    0x0,    MEM_PAGE_DEC },
        { 0x200,    0x0,    MEM_PAGE_DEC },
        { 0x2FF,    0x0,    MEM_PAGE_ASC },
        { 0x300,    0x0,    MEM_PAGE_ASC },
        { 0x400,    0x820,  0 },
};
#endif

/*
 * mem_init
 * This function initializes memory managers.
 */
void mem_init(void)
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
