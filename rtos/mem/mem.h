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
 * (in any form) the author will not be liable for any legal charges.
 */

#ifndef MEM_H
#define MEM_H

#include <config.h>

/* Memory manager configuration. */
#define CONFIG_MEMGR_STATIC
#define CONFIG_MEMGR_STATS

#ifdef CONFIG_MEMGR_STATIC
#include <mem_static.h>
#endif

/* Exported variable definitions. */
#ifdef CONFIG_MEMGR_STATIC
extern MEM_STATIC mem_static;
#endif

/* Function prototypes. */
void mem_init();

#ifdef CONFIG_MEMGR_STATIC
#define mem_static_alloc(size)      mem_static_alloc_region(&mem_static, size)
#define mem_static_dealloc(mem)     mem_static_dealloc_region(&mem_static, mem)
#endif

#endif /* MEM_H */
