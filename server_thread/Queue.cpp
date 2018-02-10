#include "Queue.h"
#include "Lock.h"
#include <assert.h>

Queue::Queue()
{
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
}

Queue::~Queue()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

int Queue::getFront()
{
    Lock l(&(this->mutex));
    while(q.empty())
    {
        pthread_cond_wait(&cond,&mutex);
    }
    assert(!q.empty());
    int connfd=q.front();
    q.pop();
    return connfd;
}

void Queue::doPush(int connfd)
{
    {
        Lock l(&(this->mutex));
        q.push(connfd);
    }
    pthread_cond_signal(&cond);
}

