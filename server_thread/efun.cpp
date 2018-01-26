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
                printf("iThisRec:%d\n",iThisRec);
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

// int readAll(int sockfd,char * buff,int len)
// {
//     int iThisRec=0;
//     int iRecLen=0;
//     while(iRecLen<len)
//     {
//         do{
//             iThisRec=read(sockfd,buff,len-iRecLen);
//         }while(iThisRec==-1 && errno==EINTR);
//         if(iThisRec<0)
//         {
//             return iThisRec;
//         }
//         else if(iThisRec==0)
//         {
//             printf("socket close\n");
//             close(sockfd);
//         }
//         iRecLen+=iThisRec;
//         buff+=iThisRec;
//     }
//     return iRecLen;
// }

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
                return iThisWri;
            }
        }
        else if(iThisWri==0)
        {
            return 0;
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

