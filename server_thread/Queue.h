#ifndef _QUEUE_H
#define _QUEUE_H

#include <queue>
#include <pthread.h>
using namespace std;

class Queue{
    queue<int> q;
    pthread_mutex_t lock;
public:
    bool isEmpty() ;
    bool getFront(int &connfd) ;
    bool doPush(int connfd);
    Queue();
    ~Queue();
};

#endif