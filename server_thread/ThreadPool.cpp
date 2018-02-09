#include "ThreadPool.h"
#include <assert.h>
#include <functional>

extern const int MAX_THREADS=64;
extern const int MAX_QUEUES=1000; 

ThreadPool::ThreadPool(int cnt):stop(false)
{
    sigset_t newset,oldset;
    sigemptyset(&newset);
    sigaddset(&newset,SIGALRM);
    int err;
    if((err=pthread_sigmask(SIG_BLOCK,&newset,&oldset))!=0)
    {
        perror("pthread_sigmask error\n");
        exit(1);
    }
    assert(cnt<=MAX_THREADS && cnt>0);
    assert(pthread_mutex_init(&p_mutex,NULL)==0 && pthread_cond_init(&p_cond,NULL)==0);
    threads.resize(cnt);
    //pthread_setconcurrency(cnt);
    for(int i=0;i<cnt;i++)
    {
        if(pthread_create(&threads[i],NULL,worker,(void *)this)!=0)
        {
            printf("create thread failed\n");
            break;
        }
    }
}

ThreadPool::~ThreadPool()
{
    {
        Lock l(&(p_mutex));
        stop=true;
        pthread_cond_broadcast(&p_cond);
    }
    for(unsigned int i=0;i<threads.size();i++)
        pthread_join(threads[i],NULL);
    threads.clear();
    pthread_mutex_destroy(&p_mutex);
    pthread_cond_destroy(&p_cond);
}

int ThreadPool::add_task(void* (*function)(void *),void * arg)
{
    Lock l(&(p_mutex));
    if(function==NULL)
        return threadpool_invalid;
    bool dosignal=false;
    if(stop)
        return threadpool_shutdown;
    if(tasks.empty())
        dosignal=true;
    tasks.push(threadpool_task(function,arg));
    if(dosignal && pthread_cond_signal(&p_cond)!=0)
        return threadpool_lock_failure;
    return 0;
}

void * ThreadPool::worker(void *arg)
{
    ThreadPool * pool=(ThreadPool *) arg;
    pool->run();
    return pool;
}

void *ThreadPool::run()
{
    while(1)
    {
        pthread_mutex_lock(&p_mutex);
        while(!stop && tasks.empty()) //当任务队列为空时一直等待,防止出现两个线程处理同一个任务的情况
            pthread_cond_wait(&p_cond,&p_mutex);
        if(stop)
            break;
        auto task=tasks.front();
        tasks.pop();
        pthread_mutex_unlock(&p_mutex);
        //printf("task.argument.connfd:%d\n",((struct Arg *)(task.argument))->connfd);
        (*(task.function))(task.argument);
    }
    //pthread_mutex_unlock(&p_mutex);
    pthread_exit(NULL);
}