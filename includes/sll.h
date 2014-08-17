#ifndef _SLL_H_
#define _SLL_H_

#include <avr/io.h>

typedef struct _sll_node
{
    void    *next;
} SLL_NODE;

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
