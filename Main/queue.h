#ifndef __QUEUE_H__
#define __QUEUE_H__


#include <stdint.h>
#include <string.h>
#include <stdlib.h>


typedef struct linklist_t
{
    void *data;
    size_t len;
    struct linklist_t *next;
} linklist_t;

typedef struct
{
    linklist_t *head;  // First mover on new item.
    linklist_t *tail;  // Point to the first data in queue.
    size_t len;
} queue_t;

extern void q_init(queue_t *q);
extern linklist_t  *q_enqueue(queue_t *q, void *data, size_t len);
extern size_t       q_dequeue(queue_t *q, void *data, size_t maxlen);
extern size_t       q_length(queue_t *q);
extern linklist_t  *q_item(queue_t *q, uint8_t index);


#endif  // __QUEUE_H__
