/*
 * mem_static.c
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
#include <string.h>

/*
 * mem_static_init
 * This function initializes a static memory pool.
 */
void mem_static_init_region(MEM_STATIC *mem_static, char *start, char *end)
{

    /* Clear the memory structure. */
    memset(mem_static, 0, sizeof(MEM_STATIC));

    /* Initialize memory pool. */
    mem_static->current_ptr = start;
    mem_static->mem_end_ptr = end;

#ifdef CONFIG_MEMGR_STATS
    mem_static->mem_size = (uint32_t)(end - start);
#endif

} /* mem_static_init */

/*
 * mem_static_alloc
 * @return: Pointer to the newly allocated memory region.
 * This function allocate a memory for a static memory pool.
 */
char *mem_static_alloc_region(MEM_STATIC *mem_static, uint32_t size)
{
    uint8_t *new_mem = NULL;

    /* Disable interrupts. */
    DISABLE_INTERRUPTS();

    if ( (mem_static->current_ptr + size) < (mem_static->mem_end_ptr) )
    {
        /* Return current memory pointer. */
        new_mem = mem_static->current_ptr;

        /* Increment the current pointer to allocate the memory. */
        mem_static->current_ptr += size;
    }

    /* Enable interrupts. */
    ENABLE_INTERRUPTS();

    /* Return allocated memory. */
    return (new_mem);

} /* mem_static_alloc_region */

/*
 * mem_static_dealloc
 * @return: If NULL memory was successfully deallocated,
 *  otherwise given memory will be returned.
 * This function allocate a memory for a static memory pool.
 */
char *mem_static_dealloc_region(MEM_STATIC *mem_static, char *mem_ptr)
{
    /* Remove some compiler warnings. */
    UNUSED_PARAM(mem_static);

    /* Return same memory as we don't support memory deallocation for
     * static memory. */
    return (mem_ptr);

} /* mem_static_alloc_region */

