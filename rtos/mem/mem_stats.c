/*
 * mem_stats.c
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
#include <serial.h>

#ifdef CONFIG_MEMGR_STATS

#ifdef CONFIG_MEMGR_DYNAMIC
/*
 * mem_dynamic_print_usage
 * @mem_dynamic: The memory region.
 * @level: Flags to specify level of required information.
 * This function will print the information about a given dynamic region.
 */
void mem_dynamic_print_usage(MEM_DYNAMIC *mem_dynamic, uint32_t level)
{
    uint32_t start, end, i, free, total_free = 0;
    MEM_FREE *free_mem;
#ifndef CONFIG_INCLUDE_SEMAPHORE
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();
#endif

#ifdef CONFIG_INCLUDE_SEMAPHORE
    /* Obtain the memory lock. */
    semaphore_obtain(&mem_dynamic->lock, MAX_WAIT);
#else
    /* Lock the scheduler. */
    DISABLE_INTERRUPTS();
#endif

    /* Memory general information.  */
    if (level & STAT_MEM_GENERAL)
    {
        start = (uint32_t)mem_dynamic->pages[0].base_start;
        end = (uint32_t)mem_dynamic->pages[mem_dynamic->num_pages - 1].base_end;

        /* Print general information about this memory region. */
        serial_printf("Memory Region Information:\r\n");
        serial_printf("Start\t\t: 0x%X\r\n", start);
        serial_printf("End\t\t: 0x%X\r\n", end);
        serial_printf("Total Size\t: %d\r\n", (end - start));
    }

    /* Page information.  */
    if ((level & STAT_MEM_PAGE_INFO) || (level & STAT_MEM_GENERAL))
    {
        /* If we are only printing page information. */
        if (!(level & STAT_MEM_GENERAL))
        {
            serial_printf("Memory Page(s) Information:\r\n");
        }

        /* If we need to print page information. */
        if (level & STAT_MEM_PAGE_INFO)
        {
            serial_printf("P[n]\tStart\t\tEnd\t\tFree\r\n");
        }

        /* Go through all the pages in this memory region. */
        for (i = 0; i < mem_dynamic->num_pages; i++)
        {
            /* Calculate free memory on this page. */
            free = 0;
            free_mem = mem_dynamic->pages[i].free_list.head;

            /* Go through free memory list. */
            while (free_mem)
            {
                free += free_mem->descriptor.size;
                free_mem = free_mem->next;
            }

            /* Add it to total free. */
            total_free += free;

            /* If we need to print per page information. */
            if (level & STAT_MEM_PAGE_INFO)
            {
                printf("[%d]\t0x%X\t0x%X\t%d\r\n", i,
                                                   mem_dynamic->pages[i].base_start,
                                                   mem_dynamic->pages[i].base_end,
                                                   free);
            }
        }

        /* Print total number of bytes free in this memory region. */
        printf("Total Free\t: %d\r\n", total_free);
    }

#ifdef CONFIG_INCLUDE_SEMAPHORE
    /* Release the memory lock. */
    semaphore_release(&mem_dynamic->lock);
#else
    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);
#endif

} /* mem_dynamic_print_usage */

#endif /* CONFIG_MEMGR_DYNAMIC */

#endif /* CONFIG_MEMGR_STATS */
