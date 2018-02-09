#include "efun.h"

int readn(int sockfd,char * buff,int len)
{
    int iThisRec=0;
    int iRecLen=0;
    while(iRecLen<len)
    {
        do{
            iThisRec=read(sockfd,buff,len-iRecLen);
        }while(iThisRec==-1 && errno==EINTR);
        if(iThisRec<0)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK)
                continue;
            else
            {
                struct sockaddr_in client_addr;
                socklen_t len;
                getsockname(sockfd,(struct sockaddr *)&client_addr,&len);
                int port=ntohs(client_addr.sin_port);
                printf("iThisRec:%d,port:%d,connfd:%d\n",iThisRec,port,sockfd);
                return iThisRec;
            }
        }
        else if(iThisRec==0)
        {
            return 0;
        }
        //printf("iThisRec:%d\n",iThisRec);
        iRecLen+=iThisRec;
        buff+=iThisRec;
    }
    return iRecLen;
}

int readNonBlock(int sockfd,char * buff,int len)
{
    int iThisRec=0;
    int iRecLen=0;
    while(iRecLen<len)
    {
        do{
            iThisRec=read(sockfd,buff,len-iRecLen);
        }while(iThisRec==-1 && errno==EINTR);
        if(iThisRec<0)
        {
            if(errno==EWOULDBLOCK)
                return iRecLen;
            else 
                return iThisRec;
        }
        else if(iThisRec==0)
        {
            return iRecLen;
        }
        iRecLen+=iThisRec;
        buff+=iThisRec;
    }
    return iRecLen;
}

int writen(int sockfd,char *buff,int len)
{
    int iThisWri;
    int iWriLen=0;
    while(iWriLen<len)
    {
        do{
            iThisWri=write(sockfd,buff,len-iWriLen);
        }while(iThisWri==-1 && errno==EINTR);
        if(iThisWri<0)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK)
                continue;
            else
            {
                printf("iThisWri:%d\n",iThisWri);
                return iWriLen;
            }
        }
        else if(iThisWri==0)
        {
            return iWriLen;
        }
        iWriLen+=iThisWri;
        buff+=iThisWri;
    }
    return iWriLen;
}

// int writeAll(int sockfd,char *buff,int len)
// {
//     int iThisWri;
//     int iWriLen=0;
//     while(iWriLen<len)
//     {
//         do{
//             iThisWri=write(sockfd,buff,len-iWriLen);
//         }while(iThisWri==-1 && errno==EINTR);
//         if(iThisWri<0)
//         {
//             return iThisWri;
//         }
//         iWriLen+=iThisWri;
//         buff+=iThisWri;
//     }
//     return iWriLen;
// }

Sigfunc *Signal(int signo,Sigfunc* func)
{
    struct sigaction act,oact;
    act.sa_handler=func;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    if(signo==SIGALRM)
    {
    #ifdef SA_INTERRUPT
        act.sa_flags|=SA_INTERRUPT;
    #endif
    }
    else {
        act.sa_flags|=SA_RESTART;
    }
    if(sigaction(signo,&act,&oact)<0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

