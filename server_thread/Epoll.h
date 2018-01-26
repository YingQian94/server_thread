#ifndef _EPOLL_H
#define _EPOLL_H

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#define FDSIZE 1000   //内核要监听的数目
#define EPOLLEVENTS 100 //等待事件产生的最大数目

class Epoll{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
public:
    Epoll();
    void add_event(int fd,int state);
    void delete_event(int fd,int state);
    void modify_event(int fd,int state);
    int get_epollfd() const {return epollfd;}
    struct epoll_event *get_epollE() {return events;}
    int get_wait(int timeout);
    ~Epoll();
};

#endif