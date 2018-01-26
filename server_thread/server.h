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

class server{
    int listenfd;
    Queue processQ;
    Queue readQ;
    Log mylog;
    Epoll myepoll;
    //map<int,shared_ptr<Data>> record; //使用shared_ptr防止内存泄漏,同时用锁进行shared_ptr的线程安全控制   
    map<int,Data> record;
    //pthread_mutex_t mapMutex;
public:
    server();
    ~server();

    void getDataK(int &k,int connfd) ;
    void getDataFilename(char *filename,int connfd) ;

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
    bool do_socket_read(int connfd);
    void do_socket_write(int connfd);
    void handle_accept();
    int get_listenfd() const {return listenfd;} 
    void setnonblock(int sockfd);
};

struct Arg{
    server *s;
    int connfd;
    Arg(server *ss,int k):s(ss),connfd(k){};
    Arg():s(NULL),connfd(-1){};
};

#endif
