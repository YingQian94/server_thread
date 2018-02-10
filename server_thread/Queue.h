#ifndef _QUEUE_H
#define _QUEUE_H

#include <queue>
#include <pthread.h>
using namespace std;

class Queue{
    queue<int> q;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
public:
    int getFront();
    void doPush(int connfd);
    Queue();
    ~Queue();
};


#endif