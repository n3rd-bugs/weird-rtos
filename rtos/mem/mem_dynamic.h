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

/* Memory page configuration flags. */
#define MEM_PAGE_DEC        0x0001
#define MEM_PAGE_ASC        0x0002

/* Memory region allocation configuration flags. */
#define MEM_STRICT_ALLOC    0x0001

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
typedef struct _mem_hole    MEM_HOLE;
typedef struct _mem_page    MEM_PAGE;
typedef struct _mem_dynamic MEM_DYNAMIC;

/* Single memory descriptor. */
typedef struct _mem_desc
{
    /* Size of this memory. */
    uint32_t        size;

    /* Physically previous node. */
    MEM_HOLE        *phy_prev;

    /* Memory page to which this memory will be returned. */
    MEM_PAGE        *page;
} MEM_DESC;

/* Single memory hole descriptor. */
struct _mem_hole
{
    /* Size of this hole. */
    uint32_t        size;

    /* Physically previous node. */
    MEM_HOLE        *phy_prev;

    /* Memory hole table. */
    MEM_HOLE        *next;
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

    /* Memory hole list. */
    struct _hole_list
    {
        MEM_HOLE    *head;
        MEM_HOLE    *tail;
    } hole_list;

    /* Biggest memory hole on this page. */
    MEM_HOLE    *max_hole;

    /* Configuration flags for this page. */
    uint32_t    flags;
};

/* Region descriptor. */
struct _mem_dynamic
{
    /* Number of pages in this memory region. */
    uint32_t        num_pages;

    /* Memory region configuration flags. */
    uint32_t        flags;

    /* Pages in this memory table. */
    MEM_PAGE        *pages;

};

/* FUnction prototypes. */
void mem_dynamic_init_region(MEM_DYNAMIC *, char *, char *, uint32_t, MEM_DYN_CFG *, uint32_t);
char *mem_dynamic_alloc_region(MEM_DYNAMIC *, uint32_t);
char *mem_dynamic_dealloc_region(char *);

#endif /* MEM_DYNAMIC_H */
