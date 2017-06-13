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
 * (in any form) the author will not be liable for any outcome from its direct
 * or indirect use.
 */
#include <os.h>
#include <mem.h>

#ifdef MEMGR_STATIC
#include <os_target.h>
#include <string.h>

/*
 * mem_static_init_region
 * @mem_static: Static memory descriptor to be initialized.
 * @start: Start of the memory.
 * @end: End of the memory.
 * This function initializes a static memory region.
 */
void mem_static_init_region(MEM_STATIC *mem_static, uint8_t *start, uint8_t *end)
{

    /* Clear the memory structure. */
    memset(mem_static, 0, sizeof(MEM_STATIC));

    /* Initialize memory region. */
    mem_static->current_ptr = start;
    mem_static->end_ptr = end;

#ifdef MEMGR_STATS
    mem_static->mem_size = (uint32_t)(end - start);
#endif

} /* mem_static_init_region */

/*
 * mem_static_alloc_region
 * @mem_static: Memory region to be used to allocate this memory.
 * @size: Size of required memory in bytes.
 * @return: Pointer to the newly allocated memory region.
 * This function allocate a memory for a static memory region.
 */
uint8_t *mem_static_alloc_region(MEM_STATIC *mem_static, uint32_t size)
{
    uint8_t *new_mem = NULL;

    /* Lock the scheduler. */
    scheduler_lock();

    if ( (mem_static->current_ptr + size) < (mem_static->end_ptr) )
    {
        /* Return current memory pointer. */
        new_mem = mem_static->current_ptr;

        /* Increment the current pointer to allocate the memory. */
        mem_static->current_ptr += size;
    }

    /* Enable scheduling. */
    scheduler_unlock();

    /* Return allocated memory. */
    return (new_mem);

} /* mem_static_alloc_region */

/*
 * mem_static_dealloc_region
 * @mem_ptr: Memory needed to be deallocated.
 * @return: If NULL memory was successfully deallocated,
 *  otherwise given memory will be returned.
 * This function allocate a memory for a static memory region.
 */
uint8_t *mem_static_dealloc_region(uint8_t *mem_ptr)
{
    /* This should never be called. */
    OS_ASSERT(TRUE);

    /* Return same memory as we don't support memory deallocation for
     * static memory. */
    return (mem_ptr);

} /* mem_static_alloc_region */

#endif /* MEMGR_STATIC */
