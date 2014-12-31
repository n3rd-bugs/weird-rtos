/*
 * mem_dynamic.c
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

/* Proof-of-concept:
 *  This memory manager works on static page scheme.
 *  Each page is initialized with a static size and a maximum allocation
 *  size.
 *  A memory can be allocated from a page if:
 *   1. The page has a hole large enough to store that memory.
 *   2. The maximum allocation size for this page is equal to or greater
 *      than the required size.
 *  These two conditions are ORed by default by can be ANDed if
 *  MEM_STRICT_ALLOC is set.
 *
 * Memory hole management:
 *  A page can be configured to work in one of the three possible
 *  configurations:
 *   1. Descending hole list, the hole list will be sorted with the largest
 *      hole first, this will work perfectly for the pages which are
 *      configured for static size allocation. Also this will require least
 *      amount of time when allocating and deallocating a memory.
 *   2. Ascending hole list, this will require more time then the last one
 *      and should be used over a smaller page delta, this will cause least
 *      fragmentation by allocating the smallest holes first.
 *   3. No sorting, this is generic configuration and will allocate the
 *      last freed memory.
 *
 * Cons:
 *  This memory management will causes more than normal fragmentation if not
 *  setup properly.
 *
 * Pros:
 *  This is highly predictable and once configured properly user will never
 *  run out of memory due to fragmentation.
 *  [No results to support this statement, this is only theoretical]
 *
 * TODO: Document test results after running them :P.
 */

#include <os.h>
#include <string.h>
#include <sll.h>

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
void mem_dynamic_init_region(MEM_DYNAMIC *mem_dynamic, char *start, char *end, uint32_t num_pages, MEM_DYN_CFG *mem_cfg, uint32_t flags)
{
    uint32_t i, j, page_mem_size, page_size, mem_size = (uint32_t)(end - start);

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
        page_mem_size = ALLIGN_CEIL(page_mem_size / j);
    }

    /* Initialize memory for all pages. */
    for (i = 0; i < num_pages; i++)
    {
        /* Initialize max memory that can be allocated for this page. */
        mem_dynamic->pages[i].max_alloc = mem_cfg[i].max_alloc + sizeof(MEM_DESC);
        mem_dynamic->pages[i].base_start = start;
        mem_dynamic->pages[i].max_hole =
        mem_dynamic->pages[i].hole_list.head =
        mem_dynamic->pages[i].hole_list.tail = (MEM_HOLE *)start;
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

            /* Move past memory for this region and assign memory boundary. */
            start += page_size;
            mem_dynamic->pages[i].base_end = start;
        }
        else
        {
            /* Just give remaining memory to last page. */
            mem_dynamic->pages[i].base_end = end;
        }

        /* Initialize memory hole list for this page. */
        mem_dynamic->pages[i].hole_list.head->phy_prev =
        mem_dynamic->pages[i].hole_list.head->next = NULL;
        mem_dynamic->pages[i].hole_list.head->size = page_size;
    }

#ifdef CONFIG_INCLUDE_SEMAPHORE
    /* Initialize memory lock. */
    /* Tasks with higher priority will be given memory first. */
    semaphore_create(&mem_dynamic->lock, 1, 1, SEMAPHORE_PRIORITY);
#endif

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
MEM_PAGE *mem_dynamic_search_region(MEM_DYNAMIC *mem_dynamic, uint32_t size, uint8_t force)
{
    MEM_PAGE *ret_page = NULL;
    uint32_t i;

    /* Go thorough all the pages in this memory region. */
    for (i = 0; i < mem_dynamic->num_pages; i++)
    {
        /* Check if we have required space on this page. */
        if ( (mem_dynamic->pages[i].max_hole) &&
             (size <= mem_dynamic->pages[i].max_hole->size) )
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
 * @node: An existing memory hole on the page.
 * @hole: New hole needed to be inserted.
 * @return: If new hole is needed to be inserted before the given hole.
 * This is hole sorting function that is used by SLL routines to maintain
 * the hole list for a page. this will maintain a page with a descending
 * memory sizes.
 */
static uint8_t mem_dynamic_sort_descending(void *node, void *hole)
{
    uint8_t insert = FALSE;

    /* If memory hole has less size than this hole then insert
     * new hole before it. */
    if (((MEM_HOLE *)node)->size < ((MEM_HOLE *)hole)->size)
    {
        /* Do insert this hole. */
        insert = TRUE;
    }

    /* Return if we need to insert this hole before the given hole. */
    return (insert);

} /* mem_dynamic_sort_descending */
/*
 * mem_dynamic_sort_ascending
 * @node: An existing memory hole on the page.
 * @hole: New hole needed to be inserted.
 * @return: If new hole is needed to be inserted before the given hole.
 * This is hole sorting function that is used by SLL routines to maintain
 * the hole list for a page. this will maintain a page with a ascending
 * memory sizes.
 */
static uint8_t mem_dynamic_sort_ascending(void *node, void *hole)
{
    uint8_t insert = FALSE;

    /* If memory hole has less greater than or equal to this hole then insert
     * new hole before it. */
    if (((MEM_HOLE *)node)->size >= ((MEM_HOLE *)hole)->size)
    {
        /* Do insert this hole. */
        insert = TRUE;
    }

    /* Return if we need to insert this hole before the given hole. */
    return (insert);

} /* mem_dynamic_sort_ascending */
/*
 * mem_dynamic_sort_none
 * @node: An existing memory hole on the page.
 * @hole: New hole needed to be inserted.
 * @return: If new hole is needed to be inserted before the given hole.
 * This is hole sorting function that is used by SLL routines to maintain
 * the hole list for a page. this is for no sorting.
 */
static uint8_t mem_dynamic_sort_none(void *node, void *hole)
{
    UNUSED_PARAM(node);
    UNUSED_PARAM(hole);

    /* Always return true to push this memory. */
    return (TRUE);

} /* mem_dynamic_sort_none */
/*
 * mem_dynamic_match_size
 * @node: An existing memory hole on the page.
 * @param: Pointer to required size.
 * @return: If new hole matches the required criteria.
 * This is match function to find a suitable hole on a memory page with given
 * size.
 */
static uint8_t mem_dynamic_match_size(void *node, void *param)
{
    uint8_t match = FALSE;

    /* Check if we can use this hole */
    if ( ((MEM_HOLE *)node)->size >= *((uint32_t *)param) )
    {
        match = TRUE;
    }

    /* Check if this hole matches the requirement. */
    return (match);

} /* mem_dynamic_match_size */

/*
 * mem_dynamic_alloc_region
 * @mem_dynamic: Dynamic memory descriptor to be used to allocate this memory.
 * @size: Size of memory to be allocated.
 * This function will allocate a memory from the giver memory region.
 */
char *mem_dynamic_alloc_region(MEM_DYNAMIC *mem_dynamic, uint32_t size)
{
    char *mem_ptr = NULL;
    MEM_PAGE *mem_page;
    MEM_HOLE *mem_hole = NULL, *max_hole;
    MEM_DESC *new_mem;
    uint32_t remaining_size;
#ifndef CONFIG_INCLUDE_SEMAPHORE
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();
#endif

    /* We will always allocate aligned memory. */
    size = ALLIGN_FLOOR(size + sizeof(MEM_DESC));

#ifdef CONFIG_INCLUDE_SEMAPHORE
    /* Acquire the memory lock. */
    semaphore_obtain(&mem_dynamic->lock, MAX_WAIT);
#else
    /* Disable global interrupts. */
    DISABLE_INTERRUPTS();
#endif

    /* First find a suitable memory page for this size. */
    mem_page = mem_dynamic_search_region(mem_dynamic, size, !(mem_dynamic->flags & MEM_STRICT_ALLOC));

    /* Check if a memory page was found that can be used to allocate this memory. */
    if (mem_page)
    {
        if (mem_page->flags & MEM_PAGE_DEC)
        {
            /* Allocate this memory on the first hole. */
            mem_hole = sll_pop(&mem_page->hole_list, OFFSETOF(MEM_HOLE, next));

            /* Update the maximum hole on this page. */
            mem_page->max_hole = mem_page->hole_list.head;
        }

        else
        {
            /* Go through all the list and find a hole that can store this memory. */
            mem_hole = sll_search_pop(&mem_page->hole_list, mem_dynamic_match_size, &size, OFFSETOF(MEM_HOLE, next));

            /* Check if this was also the biggest hole. */
            if (mem_hole == mem_page->max_hole)
            {
                /* If this page is sorted in ascending order. */
                if (mem_page->flags & MEM_PAGE_ASC)
                {
                    /* The largest memory is always at the end of the list. */
                    mem_page->max_hole = mem_page->hole_list.tail;
                }
                else
                {
                    /* Clear the max hole. */
                    mem_page->max_hole = NULL;

                    /* We will search and save max hole later. */
                }
            }
        }

        if (mem_hole)
        {
            /* If we have some remaining space on this memory hole. */
            remaining_size = mem_hole->size - size;

            /* Initialize a new memory descriptor. */
            new_mem = (MEM_DESC *)mem_hole;
            new_mem->page = mem_page;
            new_mem->size = size;

            /* No need to assign the physical previous memory. */

            /* Check if remaining memory can be defined a single memory hole. */
            if (remaining_size > sizeof(MEM_HOLE))
            {
                /* Create a new hole at the end of the allocated memory. */
                mem_hole = (MEM_HOLE *)((char *)mem_hole + size);
                mem_hole->size = remaining_size;
                mem_hole->phy_prev = (MEM_HOLE *)new_mem;

                /* Check if we need to maintain a descending list. */
                if (mem_page->flags & MEM_PAGE_DEC)
                {
                    /* Push a new hole in the list for this page in descending manner. */
                    sll_insert(&mem_page->hole_list, mem_hole, mem_dynamic_sort_descending, OFFSETOF(MEM_HOLE, next));
                }

                /* Check if we need to maintain a ascending list. */
                else if (mem_page->flags & MEM_PAGE_ASC)
                {
                    /* Push a new hole in the list for this page in ascending manner. */
                    sll_insert(&mem_page->hole_list, mem_hole, mem_dynamic_sort_ascending, OFFSETOF(MEM_HOLE, next));
                }

                /* We are not sorting the memory list. */
                else
                {
                    /* Push a new hole in the list for this page. */
                    sll_insert(&mem_page->hole_list, mem_hole, mem_dynamic_sort_none, OFFSETOF(MEM_HOLE, next));
                }
            }
            else
            {
                /* Just add the remaining memory to the original memory. */
                new_mem->size += remaining_size;
            }

            /* Return this memory. */
            mem_ptr =  (char *)(new_mem + 1);

            /* Check if we are not using any sorting. */
            if ( ((mem_page->flags & (MEM_PAGE_ASC | MEM_PAGE_DEC)) == 0) &&

                 /* Check if we need to search and update the max hole. */
                 (mem_page->max_hole == NULL) )
            {
                /* Go through all the memory holes. */
                max_hole = mem_hole = mem_page->hole_list.head;

                /* Try to find the biggest memory hole. */
                while (mem_hole)
                {
                    /* If this node has greater size then the saved node. */
                    if (mem_hole->size > max_hole->size)
                    {
                        /* Save this node. */
                        max_hole = mem_hole;
                    }

                    /* Get the next memory hole. */
                    mem_hole = mem_hole->next;
                }

                /* Update the largest hole for this page. */
                mem_page->max_hole = max_hole;
            }
        }
    }

#ifdef CONFIG_INCLUDE_SEMAPHORE
    /* Release the memory lock. */
    semaphore_release(&mem_dynamic->lock);
#else
    /* Restore old interrupt level. */
    SET_INTERRUPT_LEVEL(interrupt_level);
#endif

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
char *mem_dynamic_dealloc_region(char *mem_ptr)
{
    MEM_DESC *mem_desc;
    MEM_PAGE *mem_page;;
    MEM_DYNAMIC *mem_dynamic;
    MEM_HOLE *mem_hole, *neighbor_hole;
    uint32_t size;
    uint32_t interrupt_level = GET_INTERRUPT_LEVEL();

    /* Lock the scheduler. */
    DISABLE_INTERRUPTS();

    /* If a valid memory was given. */
    if (mem_desc)
    {
        /* Get the memory descriptor. */
        mem_desc = ((MEM_DESC *)mem_ptr) - 1;
        mem_page  = mem_desc->page;
        mem_dynamic = mem_page->mem_region;
        mem_hole = (MEM_HOLE *)mem_desc;
        size = mem_desc->size;

#ifdef CONFIG_INCLUDE_SEMAPHORE
        /* Acquire the memory lock. */
        semaphore_obtain(&mem_dynamic->lock, MAX_WAIT);

        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
#endif

        /* First initialize a new memory hole. */
        mem_hole->size = size;

        /* Process next neighbor. */
        neighbor_hole = (MEM_HOLE *)((char *)mem_hole + mem_hole->size);

        /* Check if a next node could exist after this and is also free. */
        if ( ( (char *)neighbor_hole < mem_page->base_end ) &&

             /* If this is not a memory descriptor than it's a hole. */
             ( ((MEM_DESC *)neighbor_hole)->page !=  mem_page ) )
        {
            /* Also add the size for next hole. */
            mem_hole->size += neighbor_hole->size;

            /* If there is another memory after this hole. */
            if ( ((char *)neighbor_hole + neighbor_hole->size) < mem_page->base_end )
            {
                /* Update the physical previous. */
                ((MEM_HOLE *)((char *)neighbor_hole + neighbor_hole->size))->phy_prev = mem_hole;
            }

            /* Check if this is also the max hole. */
            if (neighbor_hole == mem_page->max_hole)
            {
                /* Update the max hole. */
                mem_page->max_hole = mem_hole;
            }

            /* Remove this node from the memory list. */
            sll_remove(&mem_page->hole_list, neighbor_hole, OFFSETOF(MEM_HOLE, next));
        }

        /* Process previous neighbor. */
        neighbor_hole = mem_hole->phy_prev;

        /* Check if a node can exist before this memory and is also free. */
        if ( (neighbor_hole) &&

             /* If this is not a memory descriptor than it's a hole. */
             (((MEM_DESC *)neighbor_hole)->page !=  mem_page ) )
        {
            /* Merge this hole with previous hole. */
            neighbor_hole->size += mem_hole->size;

            /* If there is another memory after new hole. */
            if ( ((char *)neighbor_hole + neighbor_hole->size) < mem_page->base_end )
            {
                /* Update the physical previous. */
                ((MEM_HOLE *)((char *)neighbor_hole + neighbor_hole->size))->phy_prev = neighbor_hole;
            }

            /* Check if new hole is also the max hole. */
            if (mem_hole == mem_page->max_hole)
            {
                /* Update the max hole. */
                mem_page->max_hole = neighbor_hole;
            }

            /* Remove this node from the memory list. */
            sll_remove(&mem_page->hole_list, mem_hole, OFFSETOF(MEM_HOLE, next));

            /* We have merged the new hole with previous memory. */
            mem_hole = neighbor_hole;
        }

        /* If we are sorting this page in descending manner. */
        if (mem_page->flags & MEM_PAGE_DEC)
        {
            /* Push this hole on the page. */
            sll_insert(&mem_page->hole_list, mem_hole, mem_dynamic_sort_descending, OFFSETOF(MEM_HOLE, next));
        }

        /* If we are sorting this page in ascending manner. */
        else if (mem_page->flags & MEM_PAGE_ASC)
        {
            /* Push this hole on the page. */
            sll_insert(&mem_page->hole_list, mem_hole, mem_dynamic_sort_ascending, OFFSETOF(MEM_HOLE, next));
        }

        /* We are not sorting this page. */
        else
        {
            /* Push a new hole in the list for this page. */
            sll_insert(&mem_page->hole_list, mem_hole, mem_dynamic_sort_none, OFFSETOF(MEM_HOLE, next));
        }

        /* If this node has greater size then the largest hole or there is no
         * max hole in the page list. */
        if ( (mem_page->max_hole == NULL) ||
             (mem_page->max_hole->size > mem_hole->size) )
        {
            /* Update the largest hole. */
            mem_page->max_hole = mem_hole;
        }

#ifdef CONFIG_INCLUDE_SEMAPHORE
        /* Release the memory lock. */
        semaphore_release(&mem_dynamic->lock);
#else
        /* Restore old interrupt level. */
        SET_INTERRUPT_LEVEL(interrupt_level);
#endif
    }

    /* Return memory pointer. */
    return (NULL);

} /* mem_dynamic_alloc_region */
