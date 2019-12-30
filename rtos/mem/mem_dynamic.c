/*
 * mem_dynamic.c
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

/* Proof-Of-Concept:
 *  This memory manager works on static page scheme. Each page is initialized
 *  with a static size and a maximum allocation size.
 *  A memory can be allocated from a page if:
 *   1. The page has a free memory large enough to store that memory.
 *   2. The maximum allocation size for this page is equal to or greater
 *      than the required size.
 *  These two conditions are ORed by default by can be ANDed if
 *  MEM_STRICT_ALLOC is set.
 *
 * Free memory management:
 *  A page can be configured to work in one of the three possible
 *  configurations:
 *   1. Descending free list, the free list will be sorted with the largest
 *      free memory first, first memory found will be used, so it will
 *      always be the list head.
 *   2. Ascending free list, the free list will be sorted with smallest free
 *      memory first, this way always the smallest free memory that can satisfy
 *      our need will be used.
 *   3. No sorting, this is generic configuration, the list will not be sorted
 *      and first memory that can satisfy our requirement will be used.
 *
 * Per memory overhead:
 *  For each allocated memory MEM_ALOC structure will be used. In minimal
 *  configuration this will become around 12 bytes.
 *
 * Cons:
 *  This memory management will causes more than normal fragmentation, and
 *  this behavior will increase if not setup properly.
 *
 * Pros:
 *  This is highly predictable and once configured properly user can
 *  deterministically manage dynamic memory.
 *  The memory search is optimized using pages, user can define pages with
 *  same allocation size if he wants the memory to be evenly allocated.
 */
#include <kernel.h>
#include <mem.h>

#ifdef MEMGR_DYNAMIC
#include <string.h>
#include <sll.h>
#include <semaphore.h>

/* Local function prototypes. */
static MEM_PAGE *mem_dynamic_search_region(MEM_DYNAMIC *mem_dynamic, uint32_t size, uint8_t force);

/*
 * mem_dynamic_init_region
 * @mem_dynamic: The dynamic memory region descriptor to be populated.
 * @start: Start of the memory.
 * @end: End of the memory.
 * @num_page: Number of page to maintain for this region.
 * @mem_cfg: Per page configuration.
 * @flags: Memory region configuration flags.
 *  MEM_STRICT_ALLOC: A large memory will not be allocated in the
 *      small memory region.
 * This function initializes a dynamic memory region.
 */
void mem_dynamic_init_region(MEM_DYNAMIC *mem_dynamic, uint8_t *start, uint8_t *end, uint32_t num_pages, MEM_DYN_CFG *mem_cfg, uint32_t flags)
{
    uint32_t i, j, page_mem_size, page_size, mem_size = (uint32_t)(end - start);
    MEM_FREE *mem_free;

    /* Clear memory region identifier. */
    memset(mem_dynamic, 0, sizeof(MEM_DYNAMIC));

    /* Initialize memory region. */
    /* We will have page definitions on the start of the memory region. */
    mem_dynamic->pages = (MEM_PAGE *)start;
    mem_dynamic->flags = flags;
    mem_dynamic->num_pages = num_pages;

    /* Initialize memory pages. */
    memset(mem_dynamic->pages, 0, (sizeof(MEM_PAGE) * num_pages));

    /* Move past the page information. */
    start += (sizeof(MEM_PAGE) * num_pages);
    mem_size -= (sizeof(MEM_PAGE) * num_pages);

    /* Calculate undefined memory region. */
    for (i = 0, j = 0, page_mem_size = 0; i < num_pages; i++)
    {
        /* Check if page size was specified. */
        if(mem_cfg[i].size != 0)
        {
            /* Add it to total sum. */
            page_mem_size += ALLIGN_FLOOR(mem_cfg[i].size);
        }
        else
        {
            /* This is a undefined region. */
            j++;
        }
    }

    /* Calculate number of bytes for undefined regions. */
    page_mem_size = mem_size - page_mem_size;

    /* If we have any undefined region. */
    if (j > 0)
    {
        /* Evenly divide the remaining space for the memory regions that have undefined size. */
        page_mem_size = ALLIGN_FLOOR(page_mem_size / j);
    }

    /* Initialize memory for all pages. */
    for (i = 0; i < num_pages; i++)
    {
        /* Initialize max memory that can be allocated for this page. */
        mem_dynamic->pages[i].max_alloc = mem_cfg[i].max_alloc + sizeof(MEM_DESC);
#ifdef MEM_BNDRY_CHECK
        mem_dynamic->pages[i].max_alloc += (MEM_BNDRY_LENGTH * 2);
#endif
        mem_dynamic->pages[i].base_start = start;
        mem_dynamic->pages[i].flags = mem_cfg[i].flags;
        mem_dynamic->pages[i].mem_region = mem_dynamic;

        /* If this not the last page.*/
        if (i < (num_pages - 1) )
        {
            /* Check if we need to calculate memory page size or use the one
             * given by caller. */
            if (mem_cfg[i].size != 0)
            {
                page_size = ALLIGN_FLOOR(mem_cfg[i].size);
            }
            else
            {
                page_size = page_mem_size;
            }
        }
        else
        {
            /* Just give remaining memory to last page. */
            page_size = ALLIGN_FLOOR((uint32_t)(end - start));
        }

        /* Initialize a free memory. */
        mem_free = (MEM_FREE *)start;
        mem_free->descriptor.phy_prev = NULL;
        mem_free->descriptor.size = page_size;
#ifdef MEM_ID_CHECK
        mem_free->descriptor.id = MEM_FREE_ID;
#endif
        mem_free->next = NULL;

        /* Initialize free memory list for this page. */
        mem_dynamic->pages[i].free =
        mem_dynamic->pages[i].free_list.head =
        mem_dynamic->pages[i].free_list.tail = mem_free;

#ifdef MEM_FREE_CHECK
        /* Fill in the pattern that is expected if this is a free memory. */
        memset((mem_free + 1), MEM_FREE_PATTERN, page_size - sizeof(MEM_FREE));
#endif

        /* Move past assigned memory and set the boundary for this page. */
        start += page_size;
        mem_dynamic->pages[i].base_end = start;
    }

#ifdef CONFIG_SEMAPHORE
    /* Initialize memory lock. */
    /* Tasks with higher priority will be given memory first. */
    semaphore_create(&mem_dynamic->lock, 1);
#endif /* CONFIG_SEMAPHORE */

} /* mem_dynamic_init_region */

/*
 * mem_dynamic_search_region
 * @mem_dynamic: Memory region to be used to find a suitable page.
 * @size: Size of this required memory.
 * @force: Should we allocate this memory in a region with smaller
 *  maximum allocation size, if required.
 * @return: Memory page that should be used to allocate this memory.
 * This function will search for a page in which this memory can be allocated.
 */
static MEM_PAGE *mem_dynamic_search_region(MEM_DYNAMIC *mem_dynamic, uint32_t size, uint8_t force)
{
    MEM_PAGE *ret_page = NULL;
    uint32_t i;

    /* Go thorough all the pages in this memory region. */
    for (i = 0; i < mem_dynamic->num_pages; i++)
    {
        /* Check if we have required space on this page. */
        if ( (mem_dynamic->pages[i].free) &&
             (size <= mem_dynamic->pages[i].free->descriptor.size) )
        {
            /* Check if page configuration allows this allocation. */
            if (mem_dynamic->pages[i].max_alloc >= size)
            {
                /* Page found. */
                ret_page = &mem_dynamic->pages[i];

                /* Best match page. */
                break;
            }

            /* Check if we need to save a match. */
            else if (force)
            {
                /* If forced we will save this page so it can be allocated in a
                 * smaller region. */
                ret_page = &mem_dynamic->pages[i];
            }
        }
    }

    /* Return the found page that can be used for this allocation. */
    return (ret_page);

} /* mem_dynamic_search_region */

/*
 * mem_dynamic_sort_descending
 * @node: An existing free memory on the page.
 * @mem: New free memory needed to be inserted.
 * @return: If new memory is needed to be inserted before the given.
 * This is memory sorting function that is used by SLL routines to maintain
 * the memory list for a page. this will maintain a page in a descending
 * memory sizes.
 */
static uint8_t mem_dynamic_sort_descending(void *node, void *mem)
{
    uint8_t insert = FALSE;

    /* If this is larger free memory. */
    if (((MEM_DESC *)node)->size < ((MEM_DESC *)mem)->size)
    {
        /* Do insert this memory. */
        insert = TRUE;
    }

    /* Return if we need to insert this memory before the given. */
    return (insert);

} /* mem_dynamic_sort_descending */
/*
 * mem_dynamic_sort_ascending
 * @node: An existing free memory on the page.
 * @mem: New free memory needed to be inserted.
 * @return: If new memory is needed to be inserted before the given.
 * This is memory sorting function that is used by SLL routines to maintain
 * the memory list for a page. this will maintain a page in a ascending
 * memory sizes.
 */
static uint8_t mem_dynamic_sort_ascending(void *node, void *mem)
{
    uint8_t insert = FALSE;

    /* If this is smaller free memory. */
    if (((MEM_DESC *)node)->size >= ((MEM_DESC *)mem)->size)
    {
        /* Do insert this memory. */
        insert = TRUE;
    }

    /* Return if we need to insert this memory before the given. */
    return (insert);

} /* mem_dynamic_sort_ascending */

/*
 * mem_dynamic_match_size
 * @node: An existing free memory on the page.
 * @param: Pointer to required size.
 * @return: If given free memory matches the required criteria.
 * This is match function to find a suitable free memory on a page for the
 * given size.
 */
static uint8_t mem_dynamic_match_size(void *node, void *param)
{
    uint8_t match = FALSE;

#ifdef MEM_ID_CHECK
    /* Verify the free memory id. */
    ASSERT(((MEM_DESC *)node)->id != MEM_FREE_ID);
#endif

    /* Check if this memory matches the requirement. */
    if ( ((MEM_DESC *)node)->size >= *((uint32_t *)param) )
    {
        /* This free memory can be used. */
        match = TRUE;
    }

    /* Return if we can use this memory. */
    return (match);

} /* mem_dynamic_match_size */

/*
 * mem_dynamic_get_max
 * @node: An existing free memory on the page.
 * @param: Pointer to largest free memory descriptor.
 * @return: FALSE.
 * This is search function to set the largest free memory on a page.
 */
static uint8_t mem_dynamic_get_max(void *node, void *param)
{
#ifdef MEM_ID_CHECK
    /* Verify the free memory id. */
    ASSERT(((MEM_DESC *)node)->id != MEM_FREE_ID);
#endif

    /* Check if this memory has larger free memory. */
    if ( ((MEM_DESC *)node)->size >= (*(MEM_DESC **)param)->size )
    {
        /* Save this node. */
        *((MEM_DESC **)param) = node;
    }

    /* This is a search function always return false. */
    return (FALSE);

} /* mem_dynamic_get_max */

/*
 * mem_dynamic_alloc_region
 * @mem_dynamic: Dynamic memory descriptor to be used to allocate this memory.
 * @size: Size of memory to be allocated.
 * This function will allocate a memory from the given memory region.
 */
uint8_t *mem_dynamic_alloc_region(MEM_DYNAMIC *mem_dynamic, uint32_t size)
{
    uint8_t *mem_ptr = NULL;
    MEM_PAGE *mem_page;
    MEM_FREE *mem_free;
    uint32_t remaining_size;
#ifdef MEM_FREE_CHECK
    uint8_t *mem_loc, *mem_end;
#endif /* CONFIG_SEMAPHORE */

    /* We will always allocate aligned memory. */
    size = ALLIGN_CEIL(size + sizeof(MEM_ALOC));

#ifdef MEM_BNDRY_CHECK
    /* Add space for memory underflow and overflow.  */
    size += ALLIGN_CEIL(MEM_BNDRY_LENGTH * 2);
#endif

#ifdef CONFIG_SEMAPHORE
    /* Acquire the memory lock. */
    ASSERT(semaphore_obtain(&mem_dynamic->lock, MAX_WAIT) != SUCCESS);
#else
    /* Lock the scheduler. */
    scheduler_lock();
#endif /* CONFIG_SEMAPHORE */

    /* First find a suitable memory page for this size. */
    mem_page = mem_dynamic_search_region(mem_dynamic, size, !(mem_dynamic->flags & MEM_STRICT_ALLOC));

    /* Check if a memory page was found that can be used to allocate this memory. */
    if (mem_page)
    {
        if (mem_page->flags & MEM_PAGE_DEC)
        {
            /* Allocate this memory on the first free memory. */
            mem_free = sll_pop(&mem_page->free_list, OFFSETOF(MEM_FREE, next));
        }

        else
        {
            /* Go through all the list and find a free memory. */
            mem_free = sll_search_pop(&mem_page->free_list, mem_dynamic_match_size, &size, OFFSETOF(MEM_FREE, next));
        }

        /* Check if we are going to use the biggest free memory. */
        if (mem_free == mem_page->free)
        {
            /* Clear the largest free, we will search and set it later. */
            mem_page->free = NULL;
        }

        if (mem_free)
        {
#ifdef MEM_ID_CHECK
            /* Verify the free memory id. */
            ASSERT(mem_free->descriptor.id != MEM_FREE_ID);
#endif

#ifdef MEM_FREE_CHECK
            /* Verify that this memory is intact. */
            mem_loc = (uint8_t *)(mem_free + 1);
            mem_end = (uint8_t *)mem_free + mem_free->descriptor.size;

            while (mem_loc < mem_end)
            {
                ASSERT(*mem_loc != MEM_FREE_PATTERN);
                mem_loc ++;
            }
#endif

            /* If we have some remaining space in this free memory. */
            remaining_size = mem_free->descriptor.size - size;

            /* Initialize a new memory descriptor. */
            ((MEM_ALOC *)mem_free)->page = mem_page;
#ifdef MEM_ID_CHECK
            ((MEM_ALOC *)mem_free)->descriptor.id = MEM_ALLOCATED_ID;
#endif

            /* Save the memory pointer. */
            mem_ptr = (uint8_t *)mem_free;

            /* Check if remaining memory can be defined a single free memory. */
            if (remaining_size > MEM_DYN_MIN_MEM)
            {
                /* Split this memory into two parts. */
                mem_free->descriptor.size = size;

                /* Create a new free memory at the end of the allocated memory. */
                mem_free = (MEM_FREE *)((uint8_t *)mem_free + size);
                mem_free->descriptor.phy_prev = (MEM_DESC *)mem_free;
                mem_free->descriptor.size = remaining_size;
#ifdef MEM_ID_CHECK
                /* Set free memory ID for this memory. */
                mem_free->descriptor.id = MEM_FREE_ID;
#endif

                /* Check if there is a node after this memory. */
                if (((uint8_t *)mem_free + remaining_size) < mem_page->base_end)
                {
                    /* Update physical previous for next memory node. */
                    ((MEM_DESC *)(mem_free + remaining_size))->phy_prev = (MEM_DESC *)mem_free;
                }

                /* Check if we need to maintain a descending list. */
                if (mem_page->flags & MEM_PAGE_DEC)
                {
                    /* Push a new free memory in the list for this page. */
                    sll_insert(&mem_page->free_list, mem_free, mem_dynamic_sort_descending, OFFSETOF(MEM_FREE, next));
                }

                /* Check if we need to maintain a ascending list. */
                else if (mem_page->flags & MEM_PAGE_ASC)
                {
                    /* Push a new free memory in the list for this page. */
                    sll_insert(&mem_page->free_list, mem_free, mem_dynamic_sort_ascending, OFFSETOF(MEM_FREE, next));
                }

                /* We are not sorting the memory list. */
                else
                {
                    /* Push a new free memory in the list for this page. */
                    sll_push(&mem_page->free_list, mem_free, OFFSETOF(MEM_FREE, next));
                }
            }

#ifdef MEM_BNDRY_CHECK
            /* Get the memory needed to be initialized. */
            mem_free = (MEM_FREE *)mem_ptr;
#endif

            /* Adjust the pointer and return memory after the descriptor. */
            mem_ptr += sizeof(MEM_ALOC);

#ifdef MEM_BNDRY_CHECK
            /* Put a predefined pattern on memory boundaries. */
            memset(mem_ptr, 'A', mem_free->descriptor.size - sizeof(MEM_ALOC));
            memcpy(mem_ptr, MEM_BNDRY_PATTERN, MEM_BNDRY_LENGTH);
            memcpy(mem_ptr + (mem_free->descriptor.size - (MEM_BNDRY_LENGTH + sizeof(MEM_ALOC))), MEM_BNDRY_PATTERN, MEM_BNDRY_LENGTH);

            /* Adjust the pointer and return the memory after the pattern. */
            mem_ptr += MEM_BNDRY_LENGTH;
#endif

            /* Check if we need to assign largest free. */
            if (mem_page->free == NULL)
            {
                /* If this page is sorted on descending sizes. */
                if (mem_page->flags & MEM_PAGE_DEC)
                {
                    /* The largest memory is always at the start of the list. */
                    mem_page->free = mem_page->free_list.head;
                }

                /* If this page is sorted on ascending sizes. */
                else if (mem_page->flags & MEM_PAGE_ASC)
                {
                    /* The largest memory is always at the end of the list. */
                    mem_page->free = mem_page->free_list.tail;
                }

                /* If this is not a sorted page. */
                else if (!(mem_page->flags & MEM_PAGE_DEC))
                {
                    /* Initialize free memory. */
                    mem_page->free = mem_page->free_list.head;

                    /* Search and save the largest memory. */
                    sll_search_pop(&mem_page->free_list, mem_dynamic_get_max, &(mem_page->free), OFFSETOF(MEM_FREE, next));
                }
            }
        }
    }

#ifdef CONFIG_SEMAPHORE
    /* Release the memory lock. */
    semaphore_release(&mem_dynamic->lock);
#else
    /* Enable scheduling. */
    scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */

    /* Return allocated memory. */
    return (mem_ptr);

} /* mem_dynamic_alloc_region */

/*
 * mem_dynamic_dealloc_region
 * @mem_ptr: Memory needed to be deallocated.
 * @return: If NULL memory was successfully deallocated,
 *  otherwise given memory will be returned.
 * This function allocate a memory for a dynamic memory region.
 */
uint8_t *mem_dynamic_dealloc_region(uint8_t *mem_ptr)
{
    MEM_PAGE *mem_page;
    MEM_FREE *mem_free, *neighbor;

    /* Lock the scheduler. */
    scheduler_lock();

    /* If a valid memory was given. */
    if (mem_ptr)
    {
#ifdef MEM_BNDRY_CHECK
        /* Adjust the pointer to it's actual location. */
        mem_ptr -= MEM_BNDRY_LENGTH;
#endif

        /* Initialize a new free memory. */
        mem_free = ((MEM_FREE *)mem_ptr) - 1;

#ifdef CONFIG_SEMAPHORE
        /* Acquire the memory lock. */
        ASSERT(semaphore_obtain(&((MEM_ALOC *)mem_free)->page->mem_region->lock, MAX_WAIT) != SUCCESS);

        /* Enable scheduling. */
        scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */

#ifdef MEM_ID_CHECK
        /* Validate free memory id. */
        ASSERT(mem_free->descriptor.id != MEM_ALLOCATED_ID);
#endif

        /* Get memory page information from free memory descriptor. */
        mem_page  = ((MEM_ALOC *)mem_free)->page;

#ifdef MEM_BNDRY_CHECK
        /* Verify that memory boundary patterns are intact. */
        ASSERT(memcmp(mem_ptr, MEM_BNDRY_PATTERN, MEM_BNDRY_LENGTH));
        ASSERT(memcmp(mem_ptr + (mem_free->descriptor.size - (MEM_BNDRY_LENGTH + sizeof(MEM_ALOC))), MEM_BNDRY_PATTERN, MEM_BNDRY_LENGTH));
#endif
        /* Check if next memory can also be merged in this memory. */
        neighbor = (MEM_FREE *)((uint8_t *)mem_free + mem_free->descriptor.size);

        /* Check if a memory can exist after the given descriptor. */
        if ( ( (uint8_t *)neighbor < mem_page->base_end ) &&

             /* If this is not an allocated memory than it's free. */
             ( ((MEM_ALOC *)neighbor)->page !=  mem_page ) )
        {
#ifdef MEM_ID_CHECK
            /* Verify that this is a free memory. */
            ASSERT(neighbor->descriptor.id != MEM_FREE_ID);
#endif

            /* Remove neighbor from the free memory list. */
            ASSERT(sll_remove(&mem_page->free_list, neighbor, OFFSETOF(MEM_FREE, next)) != neighbor);

            /* Append neighbor to given memory. */
            mem_free->descriptor.size += neighbor->descriptor.size;

            /* If there is another memory after this. */
            if ( ((uint8_t *)mem_free + mem_free->descriptor.size) < mem_page->base_end )
            {
                /* Update the next memory. */
                ((MEM_DESC *)((uint8_t *)mem_free + mem_free->descriptor.size))->phy_prev = (MEM_DESC *)mem_free;
            }

            /* Check if neighbor is also the biggest free memory. */
            if (neighbor == mem_page->free)
            {
                /* Update the biggest free. */
                mem_page->free = mem_free;
            }
        }

        /* Process previous neighbor. */
        neighbor = (MEM_FREE *)mem_free->descriptor.phy_prev;

        /* Check if a node can exist before this memory and is also free. */
        if ( (neighbor != NULL) &&

             /* If this is not allocated than it's a free memory. */
             (((MEM_ALOC *)neighbor)->page !=  mem_page ) )
        {
#ifdef MEM_ID_CHECK
            /* Verify free id for this memory. */
            ASSERT(neighbor->descriptor.id != MEM_FREE_ID);
#endif

            /* Remove the neighbor from the free memory list. */
            ASSERT(sll_remove(&mem_page->free_list, neighbor, OFFSETOF(MEM_FREE, next)) != neighbor);

            /* If we need to update the next memory. */
            if ( ((uint8_t *)mem_free + mem_free->descriptor.size) < mem_page->base_end )
            {
                /* Update the next memory. */
                ((MEM_DESC *)((uint8_t *)mem_free + mem_free->descriptor.size))->phy_prev = (MEM_DESC *)neighbor;
            }

            /* Check if this free memory is also the largest one. */
            if (mem_free == mem_page->free)
            {
                /* Update the biggest free for this page. */
                mem_page->free = neighbor;
            }

            /* Merge new free memory in previous memory. */
            neighbor->descriptor.size += mem_free->descriptor.size;

            /* Use the neighbor as new memory has has been merged in it. */
            mem_free = neighbor;
        }

#ifdef MEM_ID_CHECK
        /* Set ID for new free memory. */
        mem_free->descriptor.id = MEM_FREE_ID;
#endif

#ifdef MEM_FREE_CHECK
        /* Fill in the pattern that is expected if this is a free memory. */
        memset((mem_free + 1), MEM_FREE_PATTERN, mem_free->descriptor.size - sizeof(MEM_FREE));
#endif

        /* If we are sorting this page in descending manner. */
        if (mem_page->flags & MEM_PAGE_DEC)
        {
            /* Push this free memory on the page. */
            sll_insert(&mem_page->free_list, mem_free, mem_dynamic_sort_descending, OFFSETOF(MEM_FREE, next));
        }

        /* If we are sorting this page in ascending manner. */
        else if (mem_page->flags & MEM_PAGE_ASC)
        {
            /* Push this free memory on the page. */
            sll_insert(&mem_page->free_list, mem_free, mem_dynamic_sort_ascending, OFFSETOF(MEM_FREE, next));
        }

        /* We are not sorting this page. */
        else
        {
            /* Push this free memory on the page. */
            sll_push(&mem_page->free_list, mem_free, OFFSETOF(MEM_FREE, next));
        }

        /* If this page has no free memory or the largest free is smaller
         * than this memory. */
        if ( (mem_page->free == NULL) ||
             (mem_free->descriptor.size > mem_page->free->descriptor.size) )
        {
            /* Update the largest free. */
            mem_page->free = mem_free;
        }

#ifdef CONFIG_SEMAPHORE
        /* Release the memory lock. */
        semaphore_release(&mem_page->mem_region->lock);
#else
        /* Enable scheduling. */
        scheduler_unlock();
#endif /* CONFIG_SEMAPHORE */
    }

    /* Return memory pointer. */
    return (NULL);

} /* mem_dynamic_alloc_region */

#endif /* MEMGR_DYNAMIC */
