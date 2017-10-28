/*
 * mem_stats.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com> All rights reserved.
 *
 * This file is part of a non-commercial software. For more details please
 * refer to the license agreement that comes with this software.
 *
 * If you have not received a license file please contact:
 *  Usama Masood <mirzaon@gmail.com>
 *
 */

#ifndef MEM_STATS_H
#define MEM_STATS_H

#include <kernel.h>

#ifdef MEMGR_STATIC

/* Information level flags. */
#define STAT_MEM_GENERAL        0x01
#define STAT_MEM_PAGE_INFO      0x02

/* Function prototypes. */
#ifdef MEMGR_DYNAMIC
void mem_dynamic_print_usage(MEM_DYNAMIC *mem_dynamic, uint32_t level);
#endif

#endif /* MEMGR_STATIC */

#endif /* MEM_STATS_H */
