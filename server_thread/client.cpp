#include "client.h"

//****************************************//
/*
客户端采用非阻塞connect+select+单进程处理多张图片的需求
*/
client::client()
{
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    maxfd=-1;
}

void client::startConnect(struct file* fptr)
{
    int flags,n;
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        perror("socket failed\n");
        exit(-1);
    }

    //设置非阻塞connect
    flags=fcntl(sockfd,F_GETFL,0);
    fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);

    struct sockaddr_in seraddr;
    seraddr.sin_family=AF_INET;
    seraddr.sin_port=htons(PORT);
    seraddr.sin_addr.s_addr=inet_addr(IPADDRESS);

    //设置接收和发送超时时间,接收超时影响5个输入函数
    //read,readv,recv,recvfrom,recvmsg
    //发送超时影响5个输出函数,write,writev,send,sendto,sendmsg
    struct timeval sendTime,revTime;
    sendTime.tv_sec=2;
    sendTime.tv_usec=0;
    revTime.tv_sec=120;
    revTime.tv_usec=0;
    if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&sendTime,sizeof(sendTime))<0)
    {
       perror("set SNDTIMEO error");
    }
    if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&revTime,sizeof(revTime))<0)
    {
        perror("set RCVTIMEO error");
    }

    fptr->f_fd=sockfd;

    if((n=connect(sockfd,(struct sockaddr *)&seraddr,sizeof(seraddr)))<0)
    {
        if(errno!=EINPROGRESS)
        {
            perror("connect failed\n");
            exit(-1);
        }
        FD_SET(sockfd,&rset);
        FD_SET(sockfd,&wset);
        fptr->f_flags=F_CONNECTING;
        if(sockfd>maxfd)
            maxfd=sockfd;
    }
    else if(n>=0)
        sendImage(fptr);
}

void client::sendImage(struct file* fptr)
{
    int k,connfd=fptr->f_fd;
    FILE *fp=fopen(fptr->f_name,"rb");
    if(fp==NULL) 
    {
        printf("filename is incorrect\n");
        return ;
    }
    fseek(fp,0,SEEK_END);
    long len=ftell(fp),needSend=len;
    //printf("len:%ld\n",len);
    len=htonl(len);
    rewind(fp);
    char buff[MAXLINE];
    memset(buff,0,MAXLINE);
    memcpy(buff,&len,sizeof(long));
    int writeLen=writen(connfd,buff,sizeof(long)); //发送文件长度
    //printf("send length l:%d\n",writeLen);
    assert(writeLen==sizeof(long));
    memset(buff,0,MAXLINE);                 
    k=htonl(fptr->k);                             
    memcpy(buff,&k,sizeof(int));
    writeLen=writen(connfd,buff,sizeof(int));         //发送k值
    //printf("send length k:%d\n",writeLen);
    assert(writeLen==sizeof(int));
    memset(buff,0,MAXLINE);
    memcpy(buff,fptr->f_name,strlen(fptr->f_name)+1);
    writeLen=writen(connfd,buff,MAXLINE);    //发送图片名
    //printf("send length imagename:%d\n",writeLen);
    assert(writeLen==MAXLINE);
    memset(buff,0,MAXLINE);
    while(needSend>MAXLINE)             //发送图片
    {
        fread((void *)buff,MAXLINE,1,fp);
        int sendLen=writen(connfd,buff,MAXLINE); 
        assert(sendLen==MAXLINE);
        memset(buff,0,MAXLINE);
        needSend-=sendLen;
    }
    if(needSend<=MAXLINE)
    {
        fread((void *)buff,needSend,1,fp);
        int sendLen=writen(connfd,buff,needSend);
        assert(sendLen==needSend);
    }
    fclose(fp);
    printf("send image success\n");
    fptr->f_flags=F_READING;
    FD_SET(fptr->f_fd,&rset);
    if(fptr->f_fd>maxfd)
        maxfd=fptr->f_fd;
}

void client::receiveImage(struct file *fptr)       //获取图片
{
    int connfd=fptr->f_fd;
    char buff[MAXLINE];int n;
    n=readn(connfd,buff,MAXLINE);
    assert(n==MAXLINE);
    char filename[NAMELEN],outname[NAMELEN];
    memcpy(filename,buff,n);
    string tmp=string(filename,strlen(filename));
    tmp=tmp.substr(tmp.find_last_of('/')+1);
    memcpy(outname,fptr->storepath,strlen(fptr->storepath));
    memcpy(outname+strlen(fptr->storepath),tmp.c_str(),strlen(tmp.c_str()));
    outname[strlen(fptr->storepath)+strlen(tmp.c_str())]='\0';
    printf("outname:%s\n",outname);
    memset(buff,0,MAXLINE);
    long len,needRead;
    n=readn(connfd,buff,sizeof(long));
    assert(n==sizeof(long));
    memcpy(&len,buff,sizeof(long));
    len=ntohl(len);
    needRead=len;
    FILE* fp=fopen(outname,"wb");
    while(needRead>MAXLINE)
    {
        n=readn(connfd,buff,MAXLINE);
        assert(n==MAXLINE);
        fwrite(buff,n,1,fp);
        memset(buff,0,MAXLINE);
        needRead-=n;
    }
    if(needRead>0)
    {
        n=readn(connfd,buff,needRead);
        assert(n==needRead);
        fwrite(buff,n,1,fp);
    }
    fclose(fp);
}

int main(int argc,char ** argv)
{
    client c;
    int nfds,fd,nfiles,maxconn,nlefttoconn,nlefttoread,nconn=0;
    int error=0;int flags;
    socklen_t len;
    fd_set rs,ws;
    if(argc<5)
    {
        printf("usage :./client conns filename storepath k filename storepath k  ... \n");
        return -1;
    }
    maxconn=atoi(argv[1]);
    nfiles=min(maxconn,MAXFILES);
    for(int i=0;i<nfiles;++i)
    {
        file[i].f_name=argv[i*3+2];
        file[i].storepath=argv[i*3+3];
        file[i].k=atoi(argv[i*3+4]);
        file[i].f_flags=0;
    }
    nlefttoread=nlefttoconn=nfiles;

    while(nlefttoread>0)
    {
        while(nconn<maxconn && nlefttoconn>0)
        {
            int i;
            for(i=0;i<nfiles;i++)       //寻找可以读的图片
            {
                if(file[i].f_flags==0)
                    break;
            }
            if(i==nfiles)
            {
                printf("nlefttoconn=%d,but nothing found.",nlefttoconn);
                exit(-1);
            }
            //printf("start connect\n");
            c.startConnect(&file[i]);
            nconn++;
            nlefttoconn--;
        }

        rs=c.rset;
        ws=c.wset;
        nfds=select(c.maxfd+1,&rs,&ws,NULL,NULL);
        if(0==nfds) continue;
        for(int i=0;i<nfiles;i++)
        {
            flags=file[i].f_flags;
            if(flags==0 || flags & F_DONE)
                continue;
            fd=file[i].f_fd;
            //当非阻塞connect连接成功时，select描述符变成可写，当连接遇到错误时，描述符变成可读又可写
            //因此使用getsockopt检查套接字上是否存在待处理错误来处理这种情形
            if(flags & F_CONNECTING && (FD_ISSET(fd,&rs)||FD_ISSET(fd,&ws)))
            {
                len=sizeof(error);
                if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&len)<0 || error!=0)
                {
                    printf("nonblock connect failed for %s:",file[i].f_name);
                    continue;
                }
                FD_CLR(fd,&(c.wset));
                c.sendImage(&file[i]);
            }
            else if(flags & F_READING && FD_ISSET(fd,&rs))
            {
                c.receiveImage(&file[i]);
                close(fd);
                printf("success got image\n");
                file[i].f_flags=F_DONE;
                FD_CLR(fd,&(c.rset));
                nconn--;
                nlefttoread--;
            }
        }
    }
    return 0;
}
