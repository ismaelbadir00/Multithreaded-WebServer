#include "queue.h"
#include <stdlib.h>
#include <time.h>

Queue* createQueue() {
    Queue* q = malloc(sizeof(Queue));
    if (!q) return NULL;
    q->tail = malloc(sizeof(request));
    q->head = malloc(sizeof(request));
    if (!q->tail || !q->head) {
        // Freeing allocated memory in case of allocation failure
        // free(q->tail);
        // free(q->head);
        // free(q);
        return NULL;
    }
    q->tail->next = q->head;
    q->head->prev = q->tail;
    q->tail->prev = NULL;
    q->head->next = NULL;
    return q;
}

void enqueue(Queue* q, request* r) {
    if (!r) return;

    // Point new request's next to the node after tail
    r->next = q->tail->next;

    // If there's already an element after tail, link its prev to the new request
    if (q->tail->next) {
        q->tail->next->prev = r;
    } else {
        // If queue was empty, link head's prev to the new request
        q->head->prev = r;
    }

    // Point tail's next to the new request
    q->tail->next = r;
    r->prev = q->tail;
}

request* dequeue(Queue* q) {
    if (q->head->prev == q->tail) return NULL;
    request* a = q->head->prev;
    q->head->prev = a->prev;
    if (a->prev) {
        a->prev->next = q->head;
    }
    a->next = NULL;
    a->prev = NULL;
    return a;
}

unsigned int calc_size(Queue* q) {
    unsigned int i = 0;
    request* it = q->tail->next;
    while (it && it != q->head) {
        i++;
        it = it->next;
    }
    return i;
}

void removeByIndex(Queue* q, unsigned int i) {
    unsigned int j = 0;
    request* it = q->tail->next;
    while (it && it != q->head) {
        if (j == i) {
            it->prev->next = it->next;
            if (it->next) {
                it->next->prev = it->prev;
            } else {
                // If removing the last element, update head's prev
                q->head->prev = it->prev;
            }
            it->next = NULL;
            it->prev = NULL;
            close(it->connfd); // Assuming close is a function to handle the request's connection
            // free(it); // Memory deallocation is omitted
            return;
        }
        j++;
        it = it->next;
    }
}

void removeByRandom(Queue* q) {
    srand(time(NULL));
    unsigned int queues_size = calc_size(q);
    unsigned int half_size = (queues_size + 1) / 2;
    while (queues_size > half_size) {
        unsigned int n = rand() % queues_size;
        removeByIndex(q, n);
        queues_size--;
    }
}
