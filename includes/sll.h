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

#include <avr/io.h>

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
void sll_push(void *list, void *node, int offset);
void *sll_pop(void *list, int offset);
void sll_append(void *list, void *node, int offset);
void sll_insert(void *list, void *node, uint8_t (*sort)(void *, void *), int offset);
void sll_remove(void *list, void *node, int offset);

#endif /* _SLL_H_ */
