/*
 * mem_stats.h
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */

#include <os.h>

#define STAT_MEM_GENERAL        0x01
#define STAT_MEM_PAGE_INFO      0x02

/* Function prototypes. */
#ifdef CONFIG_MEMGR_DYNAMIC
void mem_dynamic_print_usage(MEM_DYNAMIC *mem_dynamic, uint32_t level);
#endif
