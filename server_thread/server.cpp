#include "server.h"
#include <iostream>
#include <memory>
using namespace std;
const char DEFAULTPATH[30]="./tmpImg/";

server::server()                            //创建listenfd
{
    //pthread_mutex_init(&mapMutex,NULL);
    int sockfd;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket failed!\n");
        mylog.writeLog((char *)"socket failed!",1);
    }    
    struct sockaddr_in my_addr;
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(PORT);
    my_addr.sin_addr.s_addr=INADDR_ANY;

    //设置监听套接字为SO_REUSEADDR,服务器程序停止后想立即重启，而新套接字依旧使用同一端口
    //但必须意识到，此时任何非期望数据到达，都可能导致服务程序反应混乱，需要慎用
    int nOptval=1;                                                                          
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&nOptval,sizeof(int))<0)      
    {
        perror("set SO_REUSEADDR error\n");
        mylog.writeLog((char *)"set SO_REUSEADDR error",1);
    }
    if(bind(sockfd,(struct sockaddr*)&my_addr,sizeof(my_addr))==-1)
    {
        perror("bind failed!");
        mylog.writeLog((char *)"bind failed!",1);
    }    

    //设置套接字存活属性
    int keepalive=1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(void *)&keepalive,sizeof(keepalive))<0)
    {
        mylog.writeLog((char *)"set SO_KEEPALIVE error",1);
    }
    //设置keepalive空闲间隔时间，默认2h
    int keepalive_time=1000; 
    if(setsockopt(sockfd,IPPROTO_TCP,TCP_KEEPIDLE,(void *)&keepalive_time,sizeof(keepalive_time))<0)
    {
        mylog.writeLog((char *)"set KEEPIDLE error",1);
    }
    //设置探测消息发送频率，默认75s
    int keepalive_intvl=30;
    if(setsockopt(sockfd,IPPROTO_TCP,TCP_KEEPINTVL,(void *)&keepalive_intvl,sizeof(keepalive_intvl))<0)
    {
        mylog.writeLog((char *)"set KEEPINTVL error",1);
    }
    //设置发送探测消息次数，默认9
    int keepalive_probes=3;
    if(setsockopt(sockfd,IPPROTO_TCP,TCP_KEEPCNT,(void *)&keepalive_probes,sizeof(keepalive_probes))<0)
    {
        mylog.writeLog((char *)"set KEEPCNT error",1);
    }
    //设置tcp nodelay，不会将小包进行拼接成大包再进行发送，而是直接将小包发送出去
    int tcpnodelay=1;
    if(setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,(void *)&tcpnodelay,sizeof(tcpnodelay))<0)
    {
        mylog.writeLog((char *)"set TCP_NODELAY error",1);
    }

    if(listen(sockfd,CONNECTNUM)==-1)
    {
        perror("listen failed!");
        mylog.writeLog((char *)"listen failed!",1);
    }    
    listenfd=sockfd;
    setnonblock(listenfd);
    myepoll.add_event(listenfd,EPOLLIN|EPOLLET);    //listenfd 加入epoll
}

server::~server()
{
    close(listenfd);
    //pthread_mutex_destroy(&mapMutex);
}

void server::setnonblock(int sockfd){    //sockfd非阻塞读写，防止拒绝服务攻击
    int opts;
    opts=fcntl(sockfd,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sockfd,GETFL)");
        mylog.writeLog((char *)"fcntl(sockfd,GETFL)",1);
        exit(1) ;
    }
    opts=opts|O_NONBLOCK;
    if(fcntl(sockfd,F_SETFL,opts)<0)
    {
        perror("fcntl(sockfd,SETFL,opts)");
        mylog.writeLog((char *)"fcntl(sockfd,SETFL,opts)",1);
        exit(1);
    }
}

void server::Error(int connfd,int n)    //socket错误处理
{
    if(n==0)
    {
        printf("client close\n");
        mylog.writeLog((char *)"client close",1);
        close(connfd);
    }
    else if(n<0){
        if(!(errno==EAGAIN || errno==EWOULDBLOCK))
        {
            printf("error \n");
            mylog.writeLog((char *)"send or write error",1);
            close(connfd);
        }
    }
}

void server::handle_accept()    //处理accept新连接
{
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen=0;
    while((connfd=accept(listenfd,(struct sockaddr *)&cliaddr,&cliaddrlen))>0)
    {
        //printf("add a new client:%s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
        char printstr[NAMELEN];
        sprintf(printstr,"add a new client:%s:%d",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
        mylog.writeLog(printstr,0);
        setnonblock(connfd);
        //shared_ptr<Data> ptr(new Data);
        Data d;
        {
            // Lock(&(this->sharedMutex));
            // record[connfd]=ptr;
            //Lock(&(this->mapMutex));
            record[connfd]=d;
        }
        myepoll.add_event(connfd,EPOLLIN|EPOLLET|EPOLLONESHOT);
    }
    if(connfd<0)
    {
        if(errno!=EAGAIN && errno!=ECONNABORTED && errno!=EPROTO && errno !=EINTR)
        {
            perror("accept failed\n");
            mylog.writeLog((char *)"accept failed",1);
        }    
    }
    //myepoll.modify_event(listenfd,EPOLLIN|EPOLLET);
}

bool server::do_socket_read(int connfd) //epoll中event描述符为EPOLLIN时需要处理socket读
{
    int n;
    char buff[MAXLINE];
    //file length 读取
    {
        long l;
        memset(buff,0,MAXLINE);
        n=readn(connfd,buff,sizeof(long));
        memcpy(&l,buff,sizeof(long));
        l=(long)ntohl(l);
        if(n>0)
        {
            char printstr[NAMELEN];
            {
                //Lock(&(this->mapMutex));
                record[connfd].sockfd=connfd;
                record[connfd].fileLen=l;
                //printf("record[connfd].back().fileLen:%ld\n",record[connfd].fileLen);
                sprintf(printstr,"record[connfd].back().fileLen:%ld",record[connfd].fileLen);
            }
            mylog.writeLog(printstr,0);
        }    
        else
            Error(connfd,n);
    }
    //k 读取
    {
        int kRev;
        memset(buff,0,MAXLINE);
        n=readn(connfd,buff,sizeof(int));
        memcpy(&kRev,buff,sizeof(int));
        kRev=(int)ntohl(kRev);
        if(n>0)
        {
            char printstr[NAMELEN];
            {
                record[connfd].k=kRev;
                //printf("record[connfd].back().k:%d\n",record[connfd].k);
                sprintf(printstr,"record[connfd].back().k:%d",record[connfd].k);
            }
            mylog.writeLog(printstr,0);
        }    
        else
            Error(connfd,n);
    }
    //imagename 读取
    {
        memset(buff,0,MAXLINE);
        n=readn(connfd,buff,MAXLINE);
        if(n>0)
        {
            //Lock(&(this->mapMutex));
            memcpy(record[connfd].imagename,buff,strlen(buff)+1);
            //printf("record[connfd].back().imagename:%s\n",record[connfd].imagename);
        }
        else
            Error(connfd,n);
        char ttmp[NAMELEN],printstr[NAMELEN];
        {
            //Lock(&(this->mapMutex));
            strcpy(ttmp,record[connfd].imagename);
        }
        char *tmp;
        tmp=strrchr(ttmp,'/')+1;
        memcpy(printstr,DEFAULTPATH,strlen(DEFAULTPATH));
        struct sockaddr_in client;
        socklen_t addrlen;
        getpeername(connfd,(struct sockaddr *)&client,&addrlen);
        char addr_c[20],port_c[10],connfd_c[10];
        inet_ntop(AF_INET,&client.sin_addr.s_addr,addr_c,sizeof(addr_c));
        int port=ntohs(client.sin_port);
        sprintf(port_c,"%d",port);
        sprintf(connfd_c,"%d",connfd);
        memcpy(printstr+strlen(DEFAULTPATH),addr_c,strlen(addr_c));
        memcpy(printstr+strlen(DEFAULTPATH)+strlen(addr_c),port_c,strlen(port_c));
        memcpy(printstr+strlen(DEFAULTPATH)+strlen(addr_c)+strlen(port_c),connfd_c,strlen(connfd_c));
        memcpy(printstr+strlen(DEFAULTPATH)+strlen(addr_c)+strlen(port_c)+strlen(connfd_c),tmp,strlen(tmp));
        printstr[strlen(DEFAULTPATH)+strlen(addr_c)+strlen(port_c)+strlen(connfd_c)+strlen(tmp)]='\0';
        //printf("record[connfd].back().filename:%s\n",record[connfd].filename);
        {
            //Lock(&(this->mapMutex));
            sprintf(record[connfd].filename,"%s",printstr);
        }
        mylog.writeLog(printstr,0);
    }
    //文件读取
    {
        memset(buff,0,MAXLINE);
        char filename[NAMELEN];
        {
            //Lock(&(this->mapMutex));
            strcpy(filename,record[connfd].filename);
        }
        FILE *fp=fopen(filename,"wb");
        if(fp==NULL)
        {
            printf("open failed\n");
            return false;
        }
        long fileLen,getLen=0;
        {
            //Lock(&(this->mapMutex));
            fileLen=record[connfd].fileLen;
        }
        while(getLen<fileLen)
        {
            if((fileLen-getLen)>MAXLINE)
                n=readn(connfd,buff,MAXLINE);
            else
                n=readn(connfd,buff,fileLen-getLen);
            if(n>0)
            {
                fwrite(buff,n,1,fp);
                getLen+=n;
            }
            else if(n<0)
            {
                if(!(errno==EAGAIN || errno==EWOULDBLOCK))
                {
                    printf("error \n");
                    mylog.writeLog((char *)"send or write error",1);
                    close(connfd);
                    fclose(fp);
                    break;
                }
            }    
            else 
            {
                printf("client close\n");
                mylog.writeLog((char *)"client close",1);
                close(connfd);
                fclose(fp);
                break;
            }
            if(getLen==fileLen)
            {
                //printf("success got image\n");
                mylog.writeLog((char *)"success got image",0);
                fflush(fp);
                fclose(fp);
                doProcessPush(connfd); //加入待处理队列
                return true;
            }
        }
        return false;
    }
    //myepoll.modify_event(connfd,EPOLLIN|EPOLLET|EPOLLONESHOT); //重新加入epoll
    //return false;
}

void server::do_socket_write(int connfd) //服务器发送图片
{
    char buff[MAXLINE];
    char outname[NAMELEN],filename[NAMELEN];
    {
        //Lock(&(this->mapMutex));
        strcpy(filename,record[connfd].filename);
    }
    memcpy(outname,filename,strlen(filename)-4);
    memcpy(outname+strlen(filename)-4,"_mainColor.jpg",strlen("_mainColor.jpg"));
    outname[strlen(filename)-4+strlen("_mainColor.jpg")]='\0';
    //printf("record[%d].outname:%s\n",connfd,outname);
    mylog.writeLog(outname,0);
    int n;
    memset(buff,0,MAXLINE);
    memcpy(buff,outname,strlen(outname)+1);
    n=writen(connfd,buff,MAXLINE);       //发送 filename
    //printf("send filename finish:%d\n",n);
    if(n<=0)
        Error(connfd,n);
    memset(buff,0,MAXLINE);
    //printf("start open file\n");
    FILE *fp=fopen(outname,"rb");
    if(fp==NULL) 
    {
        printf("filename:%s\n",outname);
        printf("filename is incorrect\n");
        char printstr[NAMELEN];
        sprintf(printstr,"filename:%s is incorrect",outname);
        mylog.writeLog(printstr,1);
        return ;
    }
    // else
    //     printf("open file success\n");
    fseek(fp,0,SEEK_END);
    long len=ftell(fp),needSend=len;
    len=htonl(len);
    rewind(fp);
    //printf("needSend:%ld\n",needSend);
    memcpy(buff,&len,sizeof(long));
    int writeLen=writen(connfd,buff,sizeof(long));   //发送文件长度
    assert(writeLen==sizeof(long));
    memset(buff,0,MAXLINE);
    //printf("send length finish\n");
    while(needSend>MAXLINE)                         //发送文件
    {
        fread(buff,MAXLINE,1,fp);
        int sendLen=writen(connfd,buff,MAXLINE);
        assert(sendLen==MAXLINE);
        memset(buff,0,MAXLINE);
        needSend-=MAXLINE;
    }
    if(needSend<=MAXLINE)
    {
        fread(buff,needSend,1,fp);
        int sendLen=writen(connfd,buff,needSend);
        assert(sendLen==needSend);
    }
    fclose(fp);
    remove(outname);
    remove(filename);
    //printf("finish send image\n");
    //mylog.writeLog((char *)"finish send image",0);
    shutdown(connfd,SHUT_RD);  //无法接收数据，receive buffer 被丢弃掉，可以发送数据
    {
        //Lock(&(this->mapMutex));
        record.erase(record.find(connfd)); //从map中删除connfd
    }
    myepoll.delete_event(connfd,EPOLLOUT|EPOLLET|EPOLLONESHOT);//从epoll 中删除connfd
}

bool server::isProcessEmpty() 
{
    return processQ.isEmpty();
}

bool server::doProcessPush(int connfd) 
{
    return processQ.doPush(connfd);
}

bool server::getProcessFront(int &connfd) 
{
    return processQ.getFront(connfd);
}

bool server::isReadEmpty() 
{
    return readQ.isEmpty();
}

bool server::doReadPush(int connfd)
{
    return readQ.doPush(connfd);
}

bool server::getReadFront(int &connfd) 
{
    return readQ.getFront(connfd);
}

void server::logWrite(char *info,bool debug)
{
    mylog.writeLog(info,debug);
}

void server::logToDisk(){
    mylog.writeToDisk();
}

void server::epoll_add_event(int fd,int state){
    myepoll.add_event(fd,state);
}

void server::epoll_delete_event(int fd,int state){
    myepoll.delete_event(fd,state);
}

void server::epoll_modify_event(int fd,int state){
    myepoll.modify_event(fd,state);
}

void server::getDataK(int &k,int connfd) 
{
    //Lock(&(this->mapMutex));
    k=record[connfd].k;
}

void server::getDataFilename(char *filename,int connfd) 
{
    //Lock(&(this->mapMutex));
    strcpy(filename,record[connfd].filename);
}

int server::epoll_get_wait(int time){
    return myepoll.get_wait(time);
}

struct epoll_event *server::getEvent()
{
    return myepoll.get_epollE();
}


