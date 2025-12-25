#ifndef HW3_QUEUE_H
#define HW3_QUEUE_H
#include "segel.h"

typedef struct request{
    int connfd; // maybe no need?
    struct timeval dispatch_time;
    struct timeval arrivial_time;
    struct request* next;
    struct request* prev;
} request;

// dequeue
// enqueue
// remove by index
typedef struct Queue{
    struct request* head ;
    struct request* tail;
}Queue;

Queue* createQueue();

request* dequeue(Queue* q );

void enqueue(Queue* q, request* r);

void removeByRandom(Queue* q);

void removeByIndex(Queue* q , unsigned int i);
unsigned int calc_size(Queue* q) ;


#endif //HW3_QUEUE_H
