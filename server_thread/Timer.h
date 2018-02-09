#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>
#include <pthread.h>
#include "Lock.h"
#include "Data.h"
class server;
class heap_timer{
public:
    void (*cb_func)(server *s,int connfd);    //定时器的回调函数
    time_t expire;                              //定时器生效时间
    int connfd;
};

class time_heap
{
public:
    time_heap(int cap) ;
    time_heap(heap_timer ** init_array,int size,int capacity) ;
    ~time_heap();
    void add_timer(heap_timer *timer) ;
    void del_timer(heap_timer *timer);
    void adjust_timer(heap_timer *timer);
    void pop_timer();
    void tick(server *s);
    bool empty() const {return 0==cur_size;}
private:
    pthread_mutex_t mutex;
    void adjust_down(int hole);
    void resize();
    heap_timer ** array; //堆数组
    int capacity;       //堆数组容量
    int cur_size;       //堆数组当前包含元素的个数
};

#endif