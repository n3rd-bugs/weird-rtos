#include <sll.h>

void sll_push(void *list, void *node, int offset)
{
    /* Update the node. */
    ((SLL_NODE *)((char *)node + offset))->next = ((SLL_HEAD *)list)->head;

    /* Update the list. */
    ((SLL_HEAD *)list)->head = node;

    /* If this was an empty list. */
    if (((SLL_HEAD *)list)->tail == 0)
    {
        ((SLL_HEAD *)list)->tail = node;
    }

} /* sll_push */

void *sll_pop(void *list, int offset)
{
    void *node = ((SLL_HEAD *)list)->head;

    if (node != 0)
    {
        /* Update the list. */
        ((SLL_HEAD *)list)->head = ((SLL_NODE *)((char *)node + offset))->next;

        /* If this was the only member in the list. */
        if (((SLL_NODE *)((char *)node + offset))->next == 0)
        {
            ((SLL_HEAD *)list)->tail = 0;
        }
    }

    /* Return the head member of the list. */
    return (node);

} /* sll_pop */

void sll_append(void *list, void *node, int offset)
{
    /* Update the node. */
    ((SLL_NODE *)((char *)node + offset))->next = 0;

    if (((SLL_HEAD *)list)->tail != 0)
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

void sll_insert(void *list, void *node, uint8_t (*sort)(void *, void *), int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;

    /* If this is an empty list. */
    if ( (list_node == 0) ||

         /* If we need to insert this node on the list head. */
         (sort(list_node, node)) )
    {
        sll_push(list, node, offset);
    }
    else
    {
        /* Find the node before which we can insert this member.  */
        while ( (((SLL_NODE *)((char *)list_node + offset))->next != 0) &&
                (sort(((SLL_NODE *)((char *)list_node + offset))->next, node) == 0) )
        {
            list_node = ((SLL_NODE *)((char *)list_node + offset))->next;
        }

        /* Put this node in between the list members. */
        ((SLL_NODE *)((char *)node + offset))->next = ((SLL_NODE *)((char *)list_node + offset))->next;

        /* Check if we need to insert this node at the end of the list. */
        if (((SLL_NODE *)((char *)list_node + offset))->next == 0)
        {
            /* Put this node at the list's tail. */
            ((SLL_HEAD *)list)->tail = node;
        }

        /* Update the node before the new node. */
        ((SLL_NODE *)((char *)list_node + offset))->next = node;
    }

} /* sll_insert */

void sll_remove(void *list, void *node, int offset)
{
    void *list_node = ((SLL_HEAD *)list)->head;

    if (list_node != 0)
    {
        /* Check if we are removing a node from the list's head. */
        if (list_node == node)
        {
            /* Update the list head. */
            ((SLL_HEAD *)list)->head = ((SLL_NODE *)((char *)node + offset))->next;

            /* Check if this is the only node in the list. */
            if (((SLL_NODE *)((char *)node + offset))->next == 0)
            {
                /* Clear the list. */
                ((SLL_HEAD *)list)->tail = 0;
            }
        }

        else
        {
            /* Find the node previous to the node which we have to remove.  */
            while ( (((SLL_NODE *)((char *)list_node + offset))->next != 0) &&
                    (((SLL_NODE *)((char *)list_node + offset))->next != node) )
            {
                /* Get the next node from the list. */
                list_node = ((SLL_NODE *)((char *)list_node + offset))->next;
            }

            /* Check if we have actually found this node in the given list. */
            if (((SLL_NODE *)((char *)list_node + offset))->next != 0)
            {
                /* Update the previous entry in the link list. */
                ((SLL_NODE *)((char *)list_node + offset))->next = ((SLL_NODE *)((char *)node + offset))->next;

                /* Check if we are removing a node from the end of the list. */
                if (((SLL_NODE *)((char *)node + offset))->next == 0)
                {
                    /* We are removing a node from the tail, update the tail. */
                    ((SLL_HEAD *)list)->tail = list_node;
                }
            }
        }
    }

} /* sll_remove */
