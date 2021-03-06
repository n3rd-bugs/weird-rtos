/*
 * mem.h
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
#ifndef MEM_H
#define MEM_H

#include <kernel.h>

#ifdef CONFIG_MEMGR
#include <mem_config.h>

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
void mem_init(void);

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
