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
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>
#include <os_target.h>

#ifdef CONFIG_MEMGR_STATIC
MEM_STATIC mem_static;
#endif

/*
 * mem_init
 * This function initializes memory managers.
 */
void mem_init()
{

#ifdef CONFIG_MEMGR_STATIC
    /* Initialize global static memory pool. */
    mem_static_init_region(&mem_static, STATIC_MEM_START, STATIC_MEM_END);
#endif

} /* mem_init */



