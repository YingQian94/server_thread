#include "Queue.h"
#include "Lock.h"

Queue::Queue()
{
    pthread_mutex_init(&lock,NULL);
}

Queue::~Queue()
{
    pthread_mutex_destroy(&lock);
}

bool Queue::isEmpty() 
{
    Lock l(&(lock));
    return q.empty();
}

bool Queue::getFront(int &connfd) 
{
    Lock l(&(lock));
    if(!q.empty())
    {
        connfd=q.front();
        q.pop();
        return true;
    }
    else return false;
}

bool Queue::doPush(int connfd)
{
    Lock l(&(lock));
    q.push(connfd);
    return true;
}

