#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include "Lock.h"
#include "server.h"
using namespace std;
extern const int MAX_THREADS;
extern const int MAX_QUEUES; 

struct threadpool_task{
    void* (*function)(void *);
    void * argument;
    threadpool_task(void* (*f)(void *),void *arg):function(f),argument(arg){}
};

typedef enum{
    threadpool_invalid=-1,
    threadpool_lock_failure=-2,
    threadpool_queue_full=-3,
    threadpool_shutdown=-4,
    threadpool_thread_failure=-5
}threadpool_error_t;

class ThreadPool{
    pthread_mutex_t p_mutex;
    pthread_cond_t p_cond;
    queue<threadpool_task> tasks; //任务队列
    vector<pthread_t> threads;//可用线程
    static void *worker(void *);  //  多线程运行函数
    bool stop;                  //线程池是否终止
public:
    //typedef void*(*ThreadLoop)(void *);
    void *run();     //线程实际运行函数
    ThreadPool(int cnt=8);
    ~ThreadPool();
    int add_task(void*(*function)(void *),void * argument);
};

#endif