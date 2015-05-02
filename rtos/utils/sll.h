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
 * (in any form) the author will not be liable for any legal charges.
 */
#ifndef _SLL_H_
#define _SLL_H_

#include <stdint.h>

/* SLL configuration. */
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

/* Function prototypes. */
void sll_push(void *, void *, int);
void *sll_pop(void *, int);
void sll_append(void *, void *, int);
void sll_insert(void *, void *, uint8_t (*)(void *, void *), int);
void *sll_search(void *, void **, uint8_t (*match)(void *, void *), void *, int);
void *sll_search_pop(void *, uint8_t (*)(void *, void *), void *, int);
void sll_remove_node(void *, void *, void *, int);
void *sll_remove(void *, void *, int);
uint32_t sll_num_items(void *, int);
uint8_t sll_in_list(void *, void *, int);

#endif /* _SLL_H_ */
