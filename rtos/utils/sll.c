/*
 * sll.c
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
#include <sll.h>
#include <kernel.h>

#ifndef SLL_INLINE
/*
 * sll_push
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be added.
 * @offset: Offset of the "next" member in the node structure.
 * This function adds the given node on a list's head.
 */
void sll_push(void *list, void *node, int offset)
{
    /* A null member should never be added. */
    ASSERT(node == NULL);

#ifdef SLL_DEBUG
    /* The node should not already exist in the list. */
    ASSERT(sll_in_list(list, node, offset));
#endif

    /* Update the node. */
    ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_HEAD *)list)->head;

    /* Update the list. */
    ((SLL_HEAD *)list)->head = node;

    /* If this was an empty list. */
    if (((SLL_HEAD *)list)->tail == NULL)
    {
        ((SLL_HEAD *)list)->tail = node;
    }

} /* sll_push */

/*
 * sll_pop
 * @list: List that is needed to be updated.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Node that was removed from the list's head.
 * This function removes and returns the head node of the given list.
 */
void *sll_pop(void *list, int offset)
{
    void *node = ((SLL_HEAD *)list)->head;

    if (node != NULL)
    {
        /* Update the list. */
        ((SLL_HEAD *)list)->head = ((SLL_NODE *)((uint8_t *)node + offset))->next;

        /* If this was the only member in the list. */
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)
        {
            ((SLL_HEAD *)list)->tail = NULL;
        }
        else
        {
            /* Unlink the returned node from the list. */
            ((SLL_NODE *)((uint8_t *)node + offset))->next = NULL;
        }
    }

    /* Return the head member of the list. */
    return (node);

} /* sll_pop */

/*
 * sll_append
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be added.
 * @offset: Offset of the "next" member in the node structure.
 * This function adds a node at the end (tail) of the given list.
 */
void sll_append(void *list, void *node, int offset)
{
    /* A null member should never be added. */
    ASSERT(node == NULL);

#ifdef SLL_DEBUG
    /* The node should not already exist in the list. */
    ASSERT(sll_in_list(list, node, offset));
#endif

    /* Update the node. */
    ((SLL_NODE *)((uint8_t *)node + offset))->next = NULL;

    if (((SLL_HEAD *)list)->tail != NULL)
    {
        /* Update the existing tail in the list. */
        ((SLL_NODE *)((uint8_t *)(((SLL_HEAD *)list)->tail) + offset))->next = node;

        /* Make this node as the list's tail. */
        ((SLL_HEAD *)list)->tail = node;
    }

    /* If this was an empty list. */
    else
    {
        /* This is the only member in the list. */
        ((SLL_HEAD *)list)->head = ((SLL_HEAD *)list)->tail = node;
    }

} /* sll_append */

/*
 * sll_insert
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be added.
 * @sort: Function that will be used to sort the list. This should return TRUE
 *  if given new node (2) is needed to be placed before a particular node(1).
 * @offset: Offset of the "next" member in the node structure.
 * This function adds a new node in the given list, according to the sorting
 * criteria set by the sort function.
 */
void sll_insert(void *list, void *node, uint8_t (*sort)(void *, void *), int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;

    /* A null member should never be added. */
    ASSERT(node == NULL);

#ifdef SLL_DEBUG
    /* The node should not already exist in the list. */
    ASSERT(sll_in_list(list, node, offset));
#endif

    /* If this is an empty list. */
    if ( (list_node == NULL) ||

         /* If we need to insert this node on the list head. */
         (sort(list_node, node)) )
    {
        sll_push(list, node, offset);
    }
    else
    {
        /* Find the node before which we can insert this member.  */
        while ( (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL) &&
                (sort(((SLL_NODE *)((uint8_t *)list_node + offset))->next, node) == FALSE) )
        {
            list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;
        }

        /* Put this node in between the list members. */
        ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;

        /* Check if we need to insert this node at the end of the list. */
        if (((SLL_NODE *)((uint8_t *)list_node + offset))->next == NULL)
        {
            /* Put this node at the list's tail. */
            ((SLL_HEAD *)list)->tail = node;
        }

        /* Update the node before the new node. */
        ((SLL_NODE *)((uint8_t *)list_node + offset))->next = node;
    }

} /* sll_insert */

/*
 * sll_search
 * @list: List that is needed to be searched.
 * @prev_node: If not NULL previous node entry will be returned here.
 * @match: Function that will be called for for each node to see if this is
 *  required.
 * @offset: Offset of the "next" member in a node structure.
 * @return: If not NULL, required node will be returned.
 * This function will search for a particular node, as defined in the match
 * routine.
 */
void *sll_search(void *list, void **prev_node, uint8_t (*match)(void *, void *), void *param, int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;
    void *node = NULL;

    if (list_node != NULL)
    {
        /* Check if we need to return the first node. */
        if (match(list_node, param) == TRUE)
        {
            if (prev_node != NULL)
            {
                /* No previous entry. */
                *prev_node = NULL;
            }

            /* Return this node. */
            node = list_node;
        }

        else
        {
            while ( (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL) &&
                    (match(((SLL_NODE *)((uint8_t *)list_node + offset))->next, param) == FALSE) )
            {
                /* Get the next node in the list. */
                list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;
            }

            /* Check if we have found required node. */
            if (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL)
            {
                if (prev_node != NULL)
                {
                    /* Return previous node. */
                    *prev_node = list_node;
                }

                /* Return required node. */
                node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;
            }
        }
    }

    /* Return the required node. */
    return (node);

} /* sll_search */

/*
 * sll_search_pop
 * @list: List that is needed to be searched.
 * @match: Function that will be called with a node to see if this is the
 *  required to remove.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Node that was removed form the list.
 * This function will search for a particular node.
 */
void *sll_search_pop(void *list, uint8_t (*match)(void *, void *), void *param, int offset)
{
    void *list_node;
    void *node = sll_search(list, &list_node, match, param, offset);

    /* If a node was found. */
    if (node != NULL)
    {
        /* Remove this node. */
        sll_remove_node(list, node, list_node, offset);
    }

    /* Return the removed node. */
    return (node);

} /* sll_search_pop */

/*
 * sll_remove_node
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be removed.
 * @prev_node: Previous node.
 * @offset: Offset of the "next" member in a node structure.
 * This function removes a particular node from the given list.
 */
void sll_remove_node(void *list, void *node, void *prev_node, int offset)
{
    /* A null member should never be searched. */
    ASSERT(node == NULL);

    /* Check if we are removing a node from the list's head. */
    if (((SLL_HEAD *)list)->head == node)
    {
        /* Update the list head. */
        ((SLL_HEAD *)list)->head = ((SLL_NODE *)((uint8_t *)node + offset))->next;

        /* Check if this is the only node in the list. */
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)
        {
            /* Clear the list. */
            ((SLL_HEAD *)list)->tail = NULL;
        }
    }

    else
    {
        /* Update the previous entry in the link list. */
        ((SLL_NODE *)((uint8_t *)prev_node + offset))->next = ((SLL_NODE *)((uint8_t *)node + offset))->next;

        /* Check if we are removing a node from the end of the list. */
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)
        {
            /* We are removing a node from the tail, update the tail. */
            ((SLL_HEAD *)list)->tail = prev_node;
        }
    }

    /* Unlink this node from the list. */
    ((SLL_NODE *)((uint8_t *)node + offset))->next = NULL;

} /* sll_remove_node */

/*
 * sll_add_node
 * @list: List that is needed to be updated.
 * @node: Node needed to be added.
 * @prev_node: Node after which this node is needed to be added.
 * @offset: Offset of the "next" member in a node structure.
 * This function adds a new node in the given link list at the given location.
 */
void sll_add_node(void *list, void *node, void *prev_node, int offset)
{
    /* A null member should never be searched. */
    ASSERT(node == NULL);

#ifdef SLL_DEBUG
    /* The node should not already exist in the list. */
    ASSERT(sll_in_list(list, node, offset));
#endif

    /* Check if we are adding a node on the list's head. */
    if (prev_node == NULL)
    {
        /* Update the node. */
        ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_HEAD *)list)->head;

        /* Update the list head. */
        ((SLL_HEAD *)list)->head = node;

        /* Check if this is the first entry in the list. */
        if (((SLL_HEAD *)list)->tail == NULL)
        {
            /* Also update the tail. */
            ((SLL_HEAD *)list)->tail = node;
        }
    }

    else
    {
        /* Update the node. */
        ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_NODE *)((uint8_t *)prev_node + offset))->next;

        /* Update the previous entry in the link list. */
        ((SLL_NODE *)((uint8_t *)prev_node + offset))->next = node;

        /* Check if we are adding a node at the end of the list. */
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)
        {
            /* Also update the list tail. */
            ((SLL_HEAD *)list)->tail = node;
        }
    }

} /* sll_add_node */

/*
 * sll_remove
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be removed.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Returns node if it was removed otherwise null is returned.
 * This function removes a particular node from the given list.
 */
void *sll_remove(void *list, void *node, int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;

    /* A null member should never be removed. */
    ASSERT(node == NULL);

    /* Check if this not an empty list. */
    if (list_node != NULL)
    {
        /* Check if we are removing a node from the list's head. */
        if (list_node == node)
        {
            /* Remove this node. */
            sll_remove_node(list, node, NULL, offset);

            /* Return this node. */
            list_node = node;
        }

        else
        {
            /* Find the node previous to the node which we have to remove.  */
            while ( (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL) &&
                    (((SLL_NODE *)((uint8_t *)list_node + offset))->next != node) )
            {
                /* Get the next node from the list. */
                list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;
            }

            /* Check if we have actually found this node in the given list. */
            if (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL)
            {
                /* Remove this node. */
                sll_remove_node(list, node, list_node, offset);

                /* Return this node. */
                list_node = node;
            }

            else
            {
                /* Return null. */
                list_node = NULL;
            }
        }
    }

    /* Return the node removed. */
    return (list_node);

} /* sll_remove */

/*
 * sll_num_items
 * @list: List that is needed to be counted.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Returns the number of item in this list.
 * This function will return the number of items in a given list.
 */
uint32_t sll_num_items(void *list, int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;
    uint32_t ret_num = 0;

    /* While we have an item in this list. */
    while (list_node != NULL)
    {
        /* We have an item. */
        ret_num ++;

        /* Get the next item from the list. */
        list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;
    }

    /* Return the number of items in this list. */
    return (ret_num);

} /* sll_num_items */

/*
 * sll_in_list
 * @list: List that is needed to be searched.
 * @node: Node needed to be searched.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Returns if a member already exists in a list.
 * This function will return if a member already exists in a list.
 */
uint8_t sll_in_list(void *list, void *node, int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;
    uint8_t in_list = FALSE;

    /* While we have an item in this list. */
    while (list_node != NULL)
    {
        /* Check if this is same as the node or the list is corrupted. */
        if ((node == list_node) || (list_node == ((SLL_NODE *)((uint8_t *)list_node + offset))->next))
        {
            /* Return true. */
            in_list = TRUE;

            /* Break out of this loop. */
            break;
        }

        /* Get the next item from the list. */
        list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;
    }

    /* Return if a given node do exist in a list. */
    return (in_list);

} /* sll_in_list */

#endif /* SLL_INLINE */
