#ifndef _SERVER_H
#define _SERVER_H

#include "Data.h"
#include <map>
#include <vector>
#include <queue>
#include "efun.h"
#include "Epoll.h" 
#include "Log.h"
#include "Lock.h"
#include "Queue.h"
#include "Timer.h"

typedef void (*CBFunc)(server *s,int connfd);
extern const int TIMESLOT;

class server{
    int listenfd;
    Queue processQ;
    Queue readQ;
    Log mylog;
    Epoll myepoll;
    
    map<int,Data> record;
    pthread_mutex_t mapMutex;

    time_heap tHeap; //堆数组容量初始化为1000
   
public:
    server();
    ~server();

    void deleteRecord(int connfd);

    void getDataK(int &k,int connfd) ;
    void getDataFilename(char *filename,int connfd) ;

    void cb_func(int connfd);//定时器调用函数
    void tick();
    bool findRecord(int connfd);
    void adjust_timer(heap_timer *timer);
    heap_timer * getTimer(int connfd);

    void logWrite(char *info,bool debug);
    void logToDisk();

    void epoll_add_event(int fd,int state);
    void epoll_delete_event(int fd,int state);
    void epoll_modify_event(int fd,int state);
    int epoll_get_wait(int time);
    struct epoll_event *getEvent() ;

    bool isProcessEmpty()  ;
    bool doProcessPush(int connfd);
    bool getProcessFront(int &connfd) ;
    bool isReadEmpty()  ;
    bool doReadPush(int connfd);
    bool getReadFront(int &connfd) ;

    void Error(int connfd,int n);
    void do_socket_read(int connfd);
    void do_socket_write(int connfd);
    void handle_accept();
    int get_listenfd() const {return listenfd;} 
    void setnonblock(int sockfd);
};

#endif
