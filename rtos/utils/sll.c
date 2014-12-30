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
 * (in any form) the author will not be liable for any legal charges.
 */
#include <sll.h>
#include <os.h>

/*
 * sll_push
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be added.
 * @offset: Offset of the "next" member in the node structure.
 * This function adds the given node on a list's head.
 */
void sll_push(void *list, void *node, int offset)
{
    /* Update the node. */
    ((SLL_NODE *)((char *)node + offset))->next = ((SLL_HEAD *)list)->head;

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
        ((SLL_HEAD *)list)->head = ((SLL_NODE *)((char *)node + offset))->next;

        /* If this was the only member in the list. */
        if (((SLL_NODE *)((char *)node + offset))->next == NULL)
        {
            ((SLL_HEAD *)list)->tail = NULL;
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
    /* Update the node. */
    ((SLL_NODE *)((char *)node + offset))->next = NULL;

    if (((SLL_HEAD *)list)->tail != NULL)
    {
        /* Update the existing tail in the list. */
        ((SLL_NODE *)((char *)(((SLL_HEAD *)list)->tail) + offset))->next = node;

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
        while ( (((SLL_NODE *)((char *)list_node + offset))->next != NULL) &&
                (sort(((SLL_NODE *)((char *)list_node + offset))->next, node) == NULL) )
        {
            list_node = ((SLL_NODE *)((char *)list_node + offset))->next;
        }

        /* Put this node in between the list members. */
        ((SLL_NODE *)((char *)node + offset))->next = ((SLL_NODE *)((char *)list_node + offset))->next;

        /* Check if we need to insert this node at the end of the list. */
        if (((SLL_NODE *)((char *)list_node + offset))->next == NULL)
        {
            /* Put this node at the list's tail. */
            ((SLL_HEAD *)list)->tail = node;
        }

        /* Update the node before the new node. */
        ((SLL_NODE *)((char *)list_node + offset))->next = node;
    }

} /* sll_insert */

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
    void *node = ((SLL_HEAD *)list)->head;
    void *last_node = NULL;

    if (node != NULL)
    {
        /* Check if we need to remove the first node. */
        if ( match(node, param) )
        {
            /* Update the list head. */
            ((SLL_HEAD *)list)->head = ((SLL_NODE *)((char *)node + offset))->next;

            /* Check if this is the only node in the list. */
            if (((SLL_NODE *)((char *)node + offset))->next == NULL)
            {
                /* Clear the list. */
                ((SLL_HEAD *)list)->tail = NULL;
            }
        }

        else
        {
            /* Get the next node in the list. */
            last_node = node;
            node = ((SLL_NODE *)((char *)node + offset))->next;

            /* Go though all the nodes in this list. */
            while (node != NULL)
            {
                /* Check if need to remove this node. */
                if (match(node, param))
                {
                    /* Update the previous entry in the link list. */
                    ((SLL_NODE *)((char *)last_node + offset))->next = ((SLL_NODE *)((char *)node + offset))->next;

                    /* Check if we are removing a node from the end of the list. */
                    if (((SLL_NODE *)((char *)node + offset))->next == NULL)
                    {
                        /* We are removing a node from the tail, update the tail. */
                        ((SLL_HEAD *)list)->tail = node;
                    }
                }

                /* Get the next node in the list. */
                last_node = node;
                node = ((SLL_NODE *)((char *)node + offset))->next;
            }
        }
    }

    /* Return the removed node. */
    return (node);

} /* sll_search_pop */

/*
 * sll_remove
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be removed.
 * @offset: Offset of the "next" member in a node structure.
 * This function removes a particular node from the given list.
 */
void sll_remove(void *list, void *node, int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;

    /* Check if this not an empty list. */
    if (list_node != NULL)
    {
        /* Check if we are removing a node from the list's head. */
        if (list_node == node)
        {
            /* Update the list head. */
            ((SLL_HEAD *)list)->head = ((SLL_NODE *)((char *)node + offset))->next;

            /* Check if this is the only node in the list. */
            if (((SLL_NODE *)((char *)node + offset))->next == NULL)
            {
                /* Clear the list. */
                ((SLL_HEAD *)list)->tail = NULL;
            }
        }

        else
        {
            /* Find the node previous to the node which we have to remove.  */
            while ( (((SLL_NODE *)((char *)list_node + offset))->next != NULL) &&
                    (((SLL_NODE *)((char *)list_node + offset))->next != node) )
            {
                /* Get the next node from the list. */
                list_node = ((SLL_NODE *)((char *)list_node + offset))->next;
            }

            /* Check if we have actually found this node in the given list. */
            if (((SLL_NODE *)((char *)list_node + offset))->next != NULL)
            {
                /* Update the previous entry in the link list. */
                ((SLL_NODE *)((char *)list_node + offset))->next = ((SLL_NODE *)((char *)node + offset))->next;

                /* Check if we are removing a node from the end of the list. */
                if (((SLL_NODE *)((char *)node + offset))->next == NULL)
                {
                    /* We are removing a node from the tail, update the tail. */
                    ((SLL_HEAD *)list)->tail = list_node;
                }
            }
        }
    }

} /* sll_remove */
