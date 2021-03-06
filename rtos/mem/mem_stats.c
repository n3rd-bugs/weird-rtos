/*
 * mem_stats.c
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
#include <kernel.h>

#ifdef MEMGR_STATS
#ifdef MEMGR_DYNAMIC
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

#ifdef CONFIG_SEMAPHORE
    /* Obtain the memory lock. */
    ASSERT(semaphore_obtain(&mem_dynamic->lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif /* CONFIG_SEMAPHORE */

    /* Memory general information.  */
    if (level & STAT_MEM_GENERAL)
    {
        start = (uint32_t)mem_dynamic->pages[0].base_start;
        end = (uint32_t)mem_dynamic->pages[mem_dynamic->num_pages - 1].base_end;

        /* Print general information about this memory region. */
        printf("Memory Region Information:\r\n");
        printf("Start\t\t: 0x%X\r\n", start);
        printf("End\t\t: 0x%X\r\n", end);
        printf("Total Size\t: %d\r\n", (end - start));
    }

    /* Page information.  */
    if ((level & STAT_MEM_PAGE_INFO) || (level & STAT_MEM_GENERAL))
    {
        /* If we are only printing page information. */
        if (!(level & STAT_MEM_GENERAL))
        {
            printf("Memory Page(s) Information:\r\n");
        }

        /* If we need to print page information. */
        if (level & STAT_MEM_PAGE_INFO)
        {
            printf("P[n]\tStart\t\tEnd\t\tFree\r\n");
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

#ifdef CONFIG_SEMAPHORE
    /* Release the memory lock. */
    semaphore_release(&mem_dynamic->lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */

} /* mem_dynamic_print_usage */

#endif /* MEMGR_DYNAMIC */

#endif /* MEMGR_STATS */
