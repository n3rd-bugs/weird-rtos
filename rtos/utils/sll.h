/*
 * sll.h
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
#ifndef _SLL_H_
#define _SLL_H_

#include <stdint.h>
#include <assert.h>

/* SLL configuration. */
//#define SLL_INLINE
//#define SLL_DEBUG

/* Single SLL node definition. */
typedef struct _sll_node
{
    void    *next;
} SLL_NODE;

/* SLL list definition. */
typedef struct _sll_head
{
    void    *head;
    void    *tail;
} SLL_HEAD;

#ifdef SLL_INLINE
/*
 * sll_in_list
 * @list: List that is needed to be searched.
 * @node: Node needed to be searched.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Returns if a member already exists in a list.
 * This function will return if a member already exists in a list.
 */
inline uint8_t sll_in_list(void *list, void *node, int offset)                                          \
{                                                                                                       \
    void *list_node = ((SLL_HEAD *)list)->head;                                                         \
    uint8_t in_list = FALSE;                                                                            \
    while (list_node != NULL)                                                                           \
    {                                                                                                   \
        if ((node == list_node) || (list_node == ((SLL_NODE *)((uint8_t *)list_node + offset))->next))  \
        {                                                                                               \
            in_list = TRUE;                                                                             \
            break;                                                                                      \
        }                                                                                               \
        list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;                                \
    }                                                                                                   \
    return (in_list);                                                                                   \
} /* sll_in_list */

/*
 * sll_push
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be added.
 * @offset: Offset of the "next" member in the node structure.
 * This function adds the given node on a list's head.
 */
inline void sll_push(void *list, void *node, int offset)                                \
{                                                                                       \
    ASSERT(node == NULL);                                                            \
    ASSERT(sll_in_list(list, node, offset));                                         \
    ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_HEAD *)list)->head;          \
    ((SLL_HEAD *)list)->head = node;                                                    \
    if (((SLL_HEAD *)list)->tail == NULL)                                               \
    {                                                                                   \
        ((SLL_HEAD *)list)->tail = node;                                                \
    }                                                                                   \
} /* sll_push */

/*
 * sll_pop
 * @list: List that is needed to be updated.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Node that was removed from the list's head.
 * This function removes and returns the head node of the given list.
 */
inline void *sll_pop(void *list, int offset)                                            \
{                                                                                       \
    void *node = ((SLL_HEAD *)list)->head;                                              \
                                                                                        \
    if (node != NULL)                                                                   \
    {                                                                                   \
        ((SLL_HEAD *)list)->head = ((SLL_NODE *)((uint8_t *)node + offset))->next;      \
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)                     \
        {                                                                               \
            ((SLL_HEAD *)list)->tail = NULL;                                            \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            ((SLL_NODE *)((uint8_t *)node + offset))->next = NULL;                      \
        }                                                                               \
    }                                                                                   \
    return (node);                                                                      \
                                                                                        \
} /* sll_pop */

/*
 * sll_append
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be added.
 * @offset: Offset of the "next" member in the node structure.
 * This function adds a node at the end (tail) of the given list.
 */
inline void sll_append(void *list, void *node, int offset)                              \
{                                                                                       \
    ASSERT(node == NULL);                                                            \
    ASSERT(sll_in_list(list, node, offset));                                         \
    ((SLL_NODE *)((uint8_t *)node + offset))->next = NULL;                              \
    if (((SLL_HEAD *)list)->tail != NULL)                                               \
    {                                                                                   \
        ((SLL_NODE *)((uint8_t *)(((SLL_HEAD *)list)->tail) + offset))->next = node;    \
        ((SLL_HEAD *)list)->tail = node;                                                \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        ((SLL_HEAD *)list)->head = ((SLL_HEAD *)list)->tail = node;                     \
    }                                                                                   \
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
inline void sll_insert(void *list, void *node, uint8_t (*sort)(void *, void *), int offset)                         \
{                                                                                                                   \
    void *list_node = ((SLL_HEAD *)list)->head;                                                                     \
    ASSERT(node == NULL);                                                                                        \
    ASSERT(sll_in_list(list, node, offset));                                                                     \
    if ( (list_node == NULL) ||                                                                                     \
         (sort(list_node, node)) )                                                                                  \
    {                                                                                                               \
        sll_push(list, node, offset);                                                                               \
    }                                                                                                               \
    else                                                                                                            \
    {                                                                                                               \
        while ( (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL) &&                                    \
                (sort(((SLL_NODE *)((uint8_t *)list_node + offset))->next, node) == FALSE) )                        \
        {                                                                                                           \
            list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;                                        \
        }                                                                                                           \
        ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;       \
        if (((SLL_NODE *)((uint8_t *)list_node + offset))->next == NULL)                                            \
        {                                                                                                           \
            ((SLL_HEAD *)list)->tail = node;                                                                        \
        }                                                                                                           \
        ((SLL_NODE *)((uint8_t *)list_node + offset))->next = node;                                                 \
    }                                                                                                               \
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
inline void *sll_search(void *list, void **prev_node, uint8_t (*match)(void *, void *), void *param, int offset)    \
{                                                                                                                   \
    void *list_node = ((SLL_HEAD *)list)->head;                                                                     \
    void *node = NULL;                                                                                              \
    if (list_node != NULL)                                                                                          \
    {                                                                                                               \
        if (match(list_node, param) == TRUE)                                                                        \
        {                                                                                                           \
            if (prev_node != NULL)                                                                                  \
            {                                                                                                       \
                *prev_node = NULL;                                                                                  \
            }                                                                                                       \
            node = list_node;                                                                                       \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            while ( (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL) &&                                \
                    (match(((SLL_NODE *)((uint8_t *)list_node + offset))->next, param) == FALSE) )                  \
            {                                                                                                       \
                list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;                                    \
            }                                                                                                       \
            if (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL)                                        \
            {                                                                                                       \
                if (prev_node != NULL)                                                                              \
                {                                                                                                   \
                    *prev_node = list_node;                                                                         \
                }                                                                                                   \
                node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;                                         \
            }                                                                                                       \
        }                                                                                                           \
    }                                                                                                               \
    return (node);                                                                                                  \
} /* sll_search */

/*
 * sll_remove_node
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be removed.
 * @prev_node: Previous node.
 * @offset: Offset of the "next" member in a node structure.
 * This function removes a particular node from the given list.
 */
inline void sll_remove_node(void *list, void *node, void *prev_node, int offset)                                    \
{                                                                                                                   \
    ASSERT(node == NULL);                                                                                        \
    if (((SLL_HEAD *)list)->head == node)                                                                           \
    {                                                                                                               \
        ((SLL_HEAD *)list)->head = ((SLL_NODE *)((uint8_t *)node + offset))->next;                                  \
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)                                                 \
        {                                                                                                           \
            ((SLL_HEAD *)list)->tail = NULL;                                                                        \
        }                                                                                                           \
    }                                                                                                               \
    else                                                                                                            \
    {                                                                                                               \
        ((SLL_NODE *)((uint8_t *)prev_node + offset))->next = ((SLL_NODE *)((uint8_t *)node + offset))->next;       \
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)                                                 \
        {                                                                                                           \
            ((SLL_HEAD *)list)->tail = prev_node;                                                                   \
        }                                                                                                           \
    }                                                                                                               \
    ((SLL_NODE *)((uint8_t *)node + offset))->next = NULL;                                                          \
} /* sll_remove_node */

/*
 * sll_search_pop
 * @list: List that is needed to be searched.
 * @match: Function that will be called with a node to see if this is the
 *  required to remove.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Node that was removed form the list.
 * This function will search for a particular node.
 */
inline void *sll_search_pop(void *list, uint8_t (*match)(void *, void *), void *param, int offset)                  \
{                                                                                                                   \
    void *list_node;                                                                                                \
    void *node = sll_search(list, &list_node, match, param, offset);                                                \
    if (node != NULL)                                                                                               \
    {                                                                                                               \
        sll_remove_node(list, node, list_node, offset);                                                             \
    }                                                                                                               \
    return (node);                                                                                                  \
} /* sll_search_pop */

/*
 * sll_add_node
 * @list: List that is needed to be updated.
 * @node: Node needed to be added.
 * @prev_node: Node after which this node is needed to be added.
 * @offset: Offset of the "next" member in a node structure.
 * This function adds a new node in the given link list at the given location.
 */
inline void sll_add_node(void *list, void *node, void *prev_node, int offset)                                       \
{                                                                                                                   \
    ASSERT(node == NULL);                                                                                        \
    ASSERT(sll_in_list(list, node, offset));                                                                     \
    if (prev_node == NULL)                                                                                          \
    {                                                                                                               \
        ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_HEAD *)list)->head;                                  \
        ((SLL_HEAD *)list)->head = node;                                                                            \
        if (((SLL_HEAD *)list)->tail == NULL)                                                                       \
        {                                                                                                           \
            ((SLL_HEAD *)list)->tail = node;                                                                        \
        }                                                                                                           \
    }                                                                                                               \
    else                                                                                                            \
    {                                                                                                               \
        ((SLL_NODE *)((uint8_t *)node + offset))->next = ((SLL_NODE *)((uint8_t *)prev_node + offset))->next;       \
        ((SLL_NODE *)((uint8_t *)prev_node + offset))->next = node;                                                 \
        if (((SLL_NODE *)((uint8_t *)node + offset))->next == NULL)                                                 \
        {                                                                                                           \
            ((SLL_HEAD *)list)->tail = node;                                                                        \
        }                                                                                                           \
    }                                                                                                               \
} /* sll_add_node */

/*
 * sll_remove
 * @list: List that is needed to be updated.
 * @node: Node that is needed to be removed.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Returns node if it was removed otherwise null is returned.
 * This function removes a particular node from the given list.
 */
inline void *sll_remove(void *list, void *node, int offset)                             \
{                                                                                       \
    void *list_node = ((SLL_HEAD *)list)->head;                                         \
    ASSERT(node == NULL);                                                            \
    if (list_node != NULL)                                                              \
    {                                                                                   \
        if (list_node == node)                                                          \
        {                                                                               \
            sll_remove_node(list, node, NULL, offset);                                  \
            list_node = node;                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            while ( (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL) &&    \
                    (((SLL_NODE *)((uint8_t *)list_node + offset))->next != node) )     \
            {                                                                           \
                list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;        \
            }                                                                           \
            if (((SLL_NODE *)((uint8_t *)list_node + offset))->next != NULL)            \
            {                                                                           \
                sll_remove_node(list, node, list_node, offset);                         \
                list_node = node;                                                       \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                list_node = NULL;                                                       \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
    return (list_node);                                                                 \
} /* sll_remove */

/*
 * sll_num_items
 * @list: List that is needed to be counted.
 * @offset: Offset of the "next" member in a node structure.
 * @return: Returns the number of item in this list.
 * This function will return the number of items in a given list.
 */
inline uint32_t sll_num_items(void *list, int offset)                                   \
{                                                                                       \
    void *list_node = ((SLL_HEAD *)list)->head;                                         \
    uint32_t ret_num = 0;                                                               \
    while (list_node != NULL)                                                           \
    {                                                                                   \
        ret_num ++;                                                                     \
        list_node = ((SLL_NODE *)((uint8_t *)list_node + offset))->next;                \
    }                                                                                   \
    return (ret_num);                                                                   \
} /* sll_num_items */

#else

/* Function prototypes. */
void sll_push(void *, void *, int);
void *sll_pop(void *, int);
void sll_append(void *, void *, int);
void sll_insert(void *, void *, uint8_t (*)(void *, void *), int);
void *sll_search(void *, void **, uint8_t (*match)(void *, void *), void *, int);
void *sll_search_pop(void *, uint8_t (*)(void *, void *), void *, int);
void sll_remove_node(void *, void *, void *, int);
void sll_add_node(void *, void *, void *, int);
void *sll_remove(void *, void *, int);
uint32_t sll_num_items(void *, int);
uint8_t sll_in_list(void *, void *, int);

#endif /* SLL_INLINE */
#endif /* _SLL_H_ */
