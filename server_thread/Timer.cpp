#include "Timer.h"
#include "efun.h"

time_heap::time_heap(int cap) :capacity(cap),cur_size(0)
{
    pthread_mutex_init(&mutex,NULL);
    array=new heap_timer *[capacity];  //创建堆数组
    if(!array)
    {
        throw std::exception();
    }
    for(int i=0;i<capacity;++i)
    {
        array[i]=NULL;
    }
}

time_heap::time_heap(heap_timer ** init_array,int size,int capacity) :
   capacity(capacity), cur_size(size)
{
    pthread_mutex_init(&mutex,NULL);
    if(capacity<size)
    {
        throw std::exception();
    }
    array=new heap_timer * [capacity]; //创建堆数组
    if(!array)
    {
        throw std::exception();
    }
    for(int i=0;i<capacity;++i)
    {
        array[i]=NULL;
    }
    if(0!=size)
    {
        //初始化堆数组
        for(int i=0;i<size;++i)
        {
            array[i]=init_array[i];
        }
        for(int i=(cur_size-1)/2;i>=0;--i)
        {
            //从非叶子节点开始执行adjust_down操作
            adjust_down(i);
        }
    }
}

time_heap::~time_heap()
{
    pthread_mutex_destroy(&mutex);
    for(int i=0;i<cur_size;++i)
    {
        delete array[i];
    }
    delete []array;
}

void time_heap::adjust_timer(heap_timer *timer){
    Lock l(&(this->mutex));
    for(int i=0;i<cur_size;++i)
    {
        if(array[i]==timer)
        {
            adjust_down(i);
        }
    }
}

void time_heap::add_timer(heap_timer *timer) 
{
    if(!timer)
    {
        return;
    }
    Lock l(&(this->mutex));
    if(cur_size>=capacity)
        resize();
    int hole=cur_size++;
    int parent=0;
    for(;hole>0;hole=parent)         //adjust_up 向上调整
    {
        parent=(hole-1)/2;
        if(array[parent]->expire <= timer->expire)
        {
            break;
        }
        array[hole]=array[parent];
    }
    array[hole]=timer;
}

void time_heap::del_timer(heap_timer *timer)
{
    if(!timer)
        return;
    //仅仅将目标定时器的回调函数设置为空，即所谓的延迟销毁。
    //这将节省真正删除该定时器造成的开销，但这样做容易使堆数组膨胀
    Lock l(&(this->mutex));
    timer->cb_func=NULL;
}

//删除堆顶部的定时器
void time_heap::pop_timer()
{
    //Lock l(&(this->mutex));
    if(empty())
    {
        return;
    }
    if(array[0])
    {
        delete array[0];
        //将原来的堆顶元素替换为堆数组中最后一个元素
        array[0]=array[--cur_size];
        adjust_down(0);
    }
}

//心搏函数
void time_heap::tick(server *s)
{
    Lock l(&(this->mutex));
    heap_timer * tmp=array[0];
    time_t cur=time(NULL);
    while(!empty())
    {
        if(!tmp)
        {
            break;
        }
        if(tmp->expire > cur)
        {
            break;
        }
        if(array[0]->cb_func)
        {
            printf("cb_func::::connfd:%d\n",array[0]->connfd);//printf仅用于测试
            array[0]->cb_func(s,array[0]->connfd);
        }
        pop_timer();
        tmp=array[0];
    } 
}

void time_heap::adjust_down(int hole)
{
    /*外层调用adjust_down已经添加了锁，会出现死锁现象*/
    //Lock l(&(this->mutex));
    heap_timer *tmp=array[hole];
    int child=0;
    for(;(hole*2+1)<cur_size;hole=child)
    {
        child=hole*2+1;
        if(child < cur_size-1 && array[child+1]->expire<array[child]->expire)
            child++;
        if(array[child]->expire < tmp->expire)
            array[hole]=array[child];
        else
            break;
    }
    array[hole]=tmp;
}

//将堆数组容量扩大1倍
void time_heap::resize() 
{
    heap_timer ** tmp=new heap_timer *[2*capacity];
    for(int i=0;i<2*capacity;++i)
    {
        tmp[i]=NULL;
    }
    if(!tmp)
        throw std::exception();
    Lock l(&(this->mutex));
    capacity=2*capacity;
    for(int i=0;i<cur_size;++i)
    {
        tmp[i]=array[i];
    }
    delete [] array;
    array=tmp;
}