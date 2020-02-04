/*
 * mem_dynamic.h
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
#ifndef MEM_DYNAMIC_H
#define MEM_DYNAMIC_H

#include <kernel.h>
#include <mem.h>

#ifdef MEMGR_DYNAMIC
#include <semaphore.h>
#include <mem_dynamic_config.h>

/* Memory page configuration flags. */
#define MEM_PAGE_DEC        0x1
#define MEM_PAGE_ASC        0x2

/* Memory region allocation configuration flags. */
#define MEM_STRICT_ALLOC    0x1

#ifdef MEM_BNDRY_CHECK
#define MEM_BNDRY_PATTERN   "MMMM"
#define MEM_BNDRY_LENGTH    strlen("MMMM")
#endif

#ifdef MEM_FREE_CHECK
#define MEM_FREE_PATTERN   'F'
#endif

#ifdef MEM_ID_CHECK
#define MEM_FREE_ID         0xAABBCCDD
#define MEM_ALLOCATED_ID    0x99884433
#endif

#ifndef MEM_BNDRY_CHECK
#define MEM_DYN_MIN_MEM     (sizeof(MEM_ALOC))
#else
#define MEM_DYN_MIN_MEM     (sizeof(MEM_ALOC) + (MEM_BNDRY_LENGTH * 2))
#endif

/* Per page configuration. */
typedef struct _mem_dyn_cfg
{
    /* Maximum allocation size for this page. */
    uint32_t    max_alloc;

    /* Total size for this page. */
    uint32_t    size;

    /* Configuration flags. */
    uint32_t    flags;
} MEM_DYN_CFG;

/* Type definitions. */
typedef struct _mem_desc    MEM_DESC;
typedef struct _mem_free    MEM_FREE;
typedef struct _mem_page    MEM_PAGE;
typedef struct _mem_dynamic MEM_DYNAMIC;

/* Single memory descriptor. */
struct _mem_desc
{
#ifdef MEM_ID_CHECK
    /* ID for this memory. */
    uint32_t    id;
#endif

    /* Size of this memory. */
    uint32_t    size;

    /* Physically previous node. */
    MEM_DESC    *phy_prev;
};

/* Allocated memory descriptor. */
typedef struct _mem_allocated
{
    /* Memory descriptor. */
    MEM_DESC    descriptor;

    /* Memory page to which this memory will be returned. */
    MEM_PAGE    *page;
} MEM_ALOC;

/* Free memory descriptor. */
struct _mem_free
{
    /* Memory descriptor. */
    MEM_DESC    descriptor;

    /* Free memory list maintained according to page configuration. */
    MEM_FREE    *next;
};

/* Page descriptor. */
struct _mem_page
{
    /* Base memory definition for this page. */
    uint8_t     *base_start;
    uint8_t     *base_end;

    /* Memory region to which this page belong. */
    MEM_DYNAMIC *mem_region;

    /* Maximum allocation size for this page. */
    uint32_t    max_alloc;

    /* Free memory list. */
    struct _free_list
    {
        MEM_FREE    *head;
        MEM_FREE    *tail;
    } free_list;

    /* Largest free memory on this page. */
    MEM_FREE    *free;

    /* Configuration flags for this page. */
    uint32_t    flags;
};

/* Region descriptor. */
struct _mem_dynamic
{
#ifdef CONFIG_SEMAPHORE
    /* Memory protection lock. */
    SEMAPHORE   lock;
#endif

    /* Number of pages in this memory region. */
    uint32_t    num_pages;

    /* Memory region configuration flags. */
    uint32_t    flags;

    /* Pages in this memory table. */
    MEM_PAGE    *pages;

};

/* FUnction prototypes. */
void mem_dynamic_init_region(MEM_DYNAMIC *, uint8_t *, uint8_t *, uint32_t, MEM_DYN_CFG *, uint32_t);
uint8_t *mem_dynamic_alloc_region(MEM_DYNAMIC *, uint32_t);
uint8_t *mem_dynamic_dealloc_region(uint8_t *);

#endif /* MEMGR_DYNAMIC */
#endif /* MEM_DYNAMIC_H */
