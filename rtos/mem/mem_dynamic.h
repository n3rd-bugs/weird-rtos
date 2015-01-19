/*
 * mem_dynamic.h
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

#ifndef MEM_DYNAMIC_H
#define MEM_DYNAMIC_H

#include <os.h>

/* Dynamic memory configuration. */
#define MEM_BNDRY_CHECK
#define MEM_FREE_CHECK
#define MEM_ID_CHECK

/* Memory page configuration flags. */
#define MEM_PAGE_DEC        0x0001
#define MEM_PAGE_ASC        0x0002

/* Memory region allocation configuration flags. */
#define MEM_STRICT_ALLOC    0x0001

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
typedef struct _mem_desc
{
#ifdef MEM_ID_CHECK
    /* ID for this memory. */
    uint32_t        id;
#endif

    /* Size of this memory. */
    uint32_t        size;

    /* Physically previous node. */
    MEM_DESC        *phy_prev;
} MEM_DESC;

/* Allocated memory descriptor. */
typedef struct _mem_allocated
{
    /* Memory descriptor. */
    MEM_DESC        descriptor;

    /* Memory page to which this memory will be returned. */
    MEM_PAGE        *page;
} MEM_ALOC;

/* Free memory descriptor. */
struct _mem_free
{
    /* Memory descriptor. */
    MEM_DESC        descriptor;

    /* Free memory list maintained according to page configuration. */
    MEM_FREE        *next;
};

/* Page descriptor. */
struct _mem_page
{
    /* Base memory definition for this page. */
    char        *base_start;
    char        *base_end;

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
    SEMAPHORE       lock;
#endif

    /* Number of pages in this memory region. */
    uint32_t        num_pages;

    /* Memory region configuration flags. */
    uint32_t        flags;

    /* Pages in this memory table. */
    MEM_PAGE        *pages;

};

/* FUnction prototypes. */
void mem_dynamic_init_region(MEM_DYNAMIC *, char *, char *, uint32_t, const MEM_DYN_CFG *, uint32_t);
char *mem_dynamic_alloc_region(MEM_DYNAMIC *, uint32_t);
char *mem_dynamic_dealloc_region(char *);

#endif /* MEM_DYNAMIC_H */
