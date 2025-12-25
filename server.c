#include "segel.h"
#include "request.h"
#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// server.c: A very, very simple web server

int qsize; // current queue size
int maxqsize; // max queue size
int num_of_threads; // size of the thread array
int num_of_working_threads; // number of working threads, replacement for the second queue
Queue* q;
char* algorithm; // algorithm to use if the queue is too big

typedef struct threadInfo {
    int sttic;
    int dynamic;
    int total_req;
    int id;
} *working_thread;

pthread_cond_t multiple_cond;
pthread_cond_t size; // =cond 1
pthread_cond_t blockk; // == cond2
pthread_mutex_t LockKey;

void getargs(int *port, int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port> <num_of_threads> <max_queue_size> <algorithm>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    num_of_threads = atoi(argv[2]);
    maxqsize = atoi(argv[3]);
    algorithm = malloc(strlen(argv[4]) + 1);
    if (algorithm == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(algorithm, argv[4]);
}

void* threadHandling(void* thread) {
    working_thread th = (working_thread) thread;
    while (1) {
        pthread_mutex_lock(&LockKey);
        while (qsize == 0) {
            pthread_cond_wait(&size, &LockKey);
        }

        num_of_working_threads++;
        struct timeval dispatch_time;
        request* req = dequeue(q);
        qsize--;
        pthread_mutex_unlock(&LockKey);

        if (gettimeofday(&dispatch_time, NULL) == -1) {
            perror("gettimeofday");
            exit(EXIT_FAILURE);
        }
        int fd = req->connfd;
        timersub(&dispatch_time, &req->arrivial_time, &req->dispatch_time);
        int if_skip = 0;
        requestHandle(fd, &th->dynamic, &th->sttic, &th->total_req, th->id, &req->arrivial_time, &req->dispatch_time , &if_skip);
        Close(fd);

        if(if_skip == 1){
            pthread_mutex_lock(&LockKey);
            if(qsize !=0){
                struct timeval dispatch_time2;
                request* popped = dequeue(q);
                qsize--;
                pthread_mutex_unlock(&LockKey);
                if (gettimeofday(&dispatch_time2, NULL) == -1) {
                    perror("gettimeofday");
                    exit(EXIT_FAILURE);
                }
                int fd2 = popped->connfd;
                timersub(&dispatch_time2, &popped->arrivial_time, &popped->dispatch_time);
                if_skip = 0;
                requestHandle(fd2, &th->dynamic, &th->sttic, &th->total_req, th->id, &popped->arrivial_time, &popped->dispatch_time , &if_skip);
                // close(fd2);
            }else{
                pthread_mutex_unlock(&LockKey);
            }
        }
        pthread_mutex_lock(&LockKey);
        num_of_working_threads--;
        if (num_of_working_threads == 0 && qsize == 0) {
            pthread_cond_signal(&multiple_cond);
        }
        pthread_cond_signal(&blockk); // or block?


        pthread_mutex_unlock(&LockKey);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    getargs(&port, argc, argv);

    pthread_t threads_array[num_of_threads];
    q = createQueue();

    pthread_mutex_init(&LockKey, NULL);
    pthread_cond_init(&size, NULL);
    pthread_cond_init(&blockk, NULL);
    pthread_cond_init(&multiple_cond, NULL);

    for (int i = 0; i < num_of_threads; i++) {
        struct threadInfo* this_thread_info = malloc(sizeof(struct threadInfo));
        if (this_thread_info == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        this_thread_info->dynamic = 0;
        this_thread_info->total_req = 0;
        this_thread_info->sttic = 0;
        this_thread_info->id = i;
        int res = pthread_create(&threads_array[i], NULL, threadHandling, this_thread_info);
        if (res != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    struct timeval curr_time;

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        if (gettimeofday(&curr_time, NULL) == -1) {
            perror("gettimeofday");
            exit(EXIT_FAILURE);
        }

        pthread_mutex_lock(&LockKey);
        if (num_of_working_threads + qsize >= maxqsize) {
            if (strcmp(algorithm, "block") == 0) {
                while (num_of_working_threads + qsize >= maxqsize) {
                    pthread_cond_wait(&blockk, &LockKey);
                }

            } else if (strcmp(algorithm, "dt") == 0) {
                Close(connfd);
                pthread_mutex_unlock(&LockKey);
                continue;
            } else if (strcmp(algorithm, "dh") == 0) {
                if (qsize != 0) {
                    request* popped = dequeue(q);
                    qsize--;
                    Close(popped->connfd);
                    free(popped);
                } else {
                    Close(connfd);
                    pthread_mutex_unlock(&LockKey);
                    continue;
                }
            } else if (strcmp(algorithm, "bf") == 0) {
                while (qsize || num_of_working_threads) {
                    pthread_cond_wait(&multiple_cond, &LockKey);
                }
                Close(connfd);
                pthread_mutex_unlock(&LockKey);
                continue;
            } else if (strcmp(algorithm, "random") == 0) {
                if (qsize != 0) {
                    removeByRandom(q);
                    qsize = qsize/2;
                } else {
                    Close(connfd);
                    pthread_mutex_unlock(&LockKey);
                    continue;
                }
            }
        }

        request* r = malloc(sizeof(request));
        if (r == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        r->arrivial_time = curr_time;
        r->connfd = connfd;
        enqueue(q, r);
        qsize++;
        if (num_of_working_threads < num_of_threads) {
            pthread_cond_signal(&size);
        }
        pthread_mutex_unlock(&LockKey);
    }
}
