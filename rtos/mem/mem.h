/*
 * mem.h
 *
 * Copyright (c) 2014 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */

#ifndef MEM_H
#define MEM_H

#include <kernel.h>

#ifdef CONFIG_MEMGR

/* Memory manager configuration. */
#define MEMGR_STATIC
#define MEMGR_DYNAMIC
#define MEMGR_STATS

#ifdef MEMGR_STATIC
#include <mem_static.h>
#endif

#ifdef MEMGR_DYNAMIC
#include <mem_dynamic.h>
#endif

#ifdef MEMGR_STATS
#include <mem_stats.h>
#endif

/* Exported variable definitions. */
#ifdef MEMGR_STATIC
extern MEM_STATIC mem_static_pool;
#endif

#ifdef MEMGR_DYNAMIC
extern MEM_DYNAMIC mem_dynamic_pool;
#endif

/* Function prototypes. */
void mem_init();

#ifdef MEMGR_STATIC
#define mem_static_alloc(size)      mem_static_alloc_region(&mem_static_pool, size)
#define mem_static_dealloc(mem)     mem_static_dealloc_region((uint8_t *)mem)
#endif

#ifdef MEMGR_DYNAMIC
#define mem_dynamic_alloc(size)     mem_dynamic_alloc_region(&mem_dynamic_pool, size)
#define mem_dynamic_dealloc(mem)    mem_dynamic_dealloc_region((uint8_t *)mem)
#endif

#endif /* CONFIG_MEMGR */

#endif /* MEM_H */
