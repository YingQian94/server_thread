#include "Epoll.h"

Epoll::Epoll()
{
    epollfd=epoll_create(FDSIZE);
}

Epoll::~Epoll()
{
    close(epollfd);
}

void Epoll::add_event(int fd,int state)
{
    struct epoll_event ev;
    ev.events=state;
    ev.data.fd=fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void Epoll::delete_event(int fd,int state)
{
    struct epoll_event ev;
    ev.events=state;
    ev.data.fd=fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

void Epoll::modify_event(int fd,int state)
{
    struct epoll_event ev;
    ev.events=state;
    ev.data.fd=fd;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

int Epoll::get_wait(int timeout)
{
    return epoll_wait(epollfd,events,EPOLLEVENTS,timeout);
}
