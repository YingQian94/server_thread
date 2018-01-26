#include "server.h"
#include "ThreadPool.h"
#include <vector>
#include <time.h>
#include <iostream>
#include <algorithm>
#include <signal.h>

/****************************/
/*忽略SIGPIPE，当客户端断开连接后而本地继续写入会触发SIGPIPE，
SIGPIPE,默认终止进程，会造成服务器意外退出
全局域只能声明，初始化变量，不能对变量进行赋值，运算，调用函数等操作，
因此使用c++全局对象*/
/***************************/
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        Signal(SIGPIPE,SIG_IGN);
    }
};

IgnoreSigPipe initObj;

Arg args[1000];  //用于传递给线程池中的参数，防止在同一地址进行取值，造成取值不同的错误

ThreadPool pool(8);//线程池有8个线程

void * k_means(Arg arg);//使用k-means得到图片主颜色
void *handleread(void *arg);        //处理读线程
void * processImage(void *arg);  //处理图片线程
void *logThread(void *arg);  //处理日志线程


int main()
{
    server s;//服务器初始化
    pthread_t pid[3];
    pthread_create(&pid[0],NULL,handleread,&s);
    pthread_create(&pid[1],NULL,processImage,&s);
    pthread_create(&pid[2],NULL,logThread,&s);
    while(1)
    {
        int num=s.epoll_get_wait(-1),fd;   //-1表示epoll无限等待
        struct epoll_event *events=s.getEvent();
        int listenfd=s.get_listenfd();
        if (-1 == num) {
            if (errno == EINTR) {
                continue;
            }
            printf("epoll wait error.");
            s.logWrite((char *)"epoll wait error.",1);
            return -1;
        }
        for(int i=0;i<num;i++)
        {
            fd=events[i].data.fd;
            if((fd==listenfd)&& (events[i].events & EPOLLIN))
            {
                s.handle_accept();
            }
            else if(events[i].events & EPOLLIN)
            {
                s.doReadPush(fd);
            }
            else if(events[i].events & EPOLLOUT)
            {
                s.do_socket_write(fd);
            }
        }
    }
    for(int i=0;i<3;i++)
    {
        pthread_join(pid[i],NULL);
    }
    exit(0);
}

void * k_means(void *arg)
{
    Arg *a=(Arg *)arg;
    server *s=a->s;
    int connfd=a->connfd;
    a->connfd=-1;
    s->logWrite((char *)"k_means start",0);
    Mat small,image;
    int k;s->getDataK(k,connfd);
    char imagename[NAMELEN];
    s->getDataFilename(imagename,connfd);
    //printf("connfd:%d,k:%d,imagename:%s\n",connfd,k,imagename);
    image=imread(imagename);
    int r=(int)image.rows/10,c=(int)image.cols/10;
    resize(image,small,Size(r,c),0,0,INTER_AREA);
    vector<vector<int>> k_color(k,vector<int>(3));  //中心点rgb值
    const int MAXSTEP=100;                          //最大迭代次数100
    int change=0;                                   //记录k个中心点是否收敛
    srand((unsigned)time(NULL));
    for(int i=0;i<k;i++)                            //初始化k个中心点
    {
        int x_pos,y_pos;
        x_pos=rand()%small.rows;
        y_pos=rand()%small.cols;
        k_color[i][0]=(int)small.at<Vec3b>(x_pos,y_pos)[0];
        k_color[i][1]=(int)small.at<Vec3b>(x_pos,y_pos)[1];
        k_color[i][2]=(int)small.at<Vec3b>(x_pos,y_pos)[2];
    }
    int step=0;
    while(step<MAXSTEP && change<k)
    {
        step++;
        change=0;
        vector<vector<pair<int,int>>> KGROUPS;      //用于存放k种聚类后颜色的坐标点构成
        KGROUPS.resize(k);
        for(int i=0;i<small.rows;i++)               //每个点加入距离最短的group
            for(int j=0;j<small.cols;j++)
            {
                vector<int> dis;
                for(int r=0;r<k;r++)
                    dis.push_back(abs((int)small.at<Vec3b>(i,j)[0]-k_color[r][0])+
                abs((int)small.at<Vec3b>(i,j)[1]-k_color[r][1])+
                abs((int)small.at<Vec3b>(i,j)[2]-k_color[r][2]));
                int min_index=-1,min_value=999999;
                for(int r=0;r<k;r++)
                    if(dis[r]<min_value)
                    {
                        min_value=dis[r];
                        min_index=r;
                    }
                KGROUPS[min_index].push_back({i,j});
            }
        for(int r=0;r<k;r++)                        //重新计算k个类的中心点
        {
            int B_color=0,G_color=0,R_color=0;
            if(KGROUPS[r].size()==0)                //某一类没有分配到与他最近的中心点，则随机初始化
            {
                k_color[r][0]=rand()%256;
                k_color[r][1]=rand()%256;
                k_color[r][2]=rand()%256;
            }
            else{
                for(unsigned int i=0;i<KGROUPS[r].size();i++)
                {
                    B_color+=(int)small.at<Vec3b>(KGROUPS[r][i].first,KGROUPS[r][i].second)[0];
                    G_color+=(int)small.at<Vec3b>(KGROUPS[r][i].first,KGROUPS[r][i].second)[1];
                    R_color+=(int)small.at<Vec3b>(KGROUPS[r][i].first,KGROUPS[r][i].second)[2];
                }
                B_color/=KGROUPS[r].size();
                G_color/=KGROUPS[r].size();
                R_color/=KGROUPS[r].size();
                if(B_color==k_color[r][0] && G_color==k_color[r][1] && R_color==k_color[r][2])
                    change+=1;
                k_color[r][0]=B_color;
                k_color[r][1]=G_color;
                k_color[r][2]=R_color;
            }
        }
    }
    char outname[NAMELEN],filename[NAMELEN];
    s->getDataFilename(filename,connfd);
    memcpy(outname,filename,strlen(filename)-4);
    memcpy(outname+strlen(filename)-4,"_mainColor.jpg",strlen("_mainColor.jpg"));
    outname[strlen(filename)-4+strlen("_mainColor.jpg")]='\0';
    int writeStep=(int)small.rows/k;                  //输出主颜色到图片
    for(int r=0;r<k;r++)
    {
        int end=(r==k-1)?small.rows:writeStep*(r+1);
        for(int i=writeStep*r;i<end;i++)
            for(int j=0;j<small.cols;j++)
            {
                small.at<Vec3b>(i,j)[0]=k_color[r][0];
                small.at<Vec3b>(i,j)[1]=k_color[r][1];
                small.at<Vec3b>(i,j)[2]=k_color[r][2];
            }
    }
    // printf("k_means_outname:%s\n",outname);
    imwrite(outname,small);
    //printf("k_means end\n");
    s->logWrite((char *)"k_means end",0);
    s->epoll_modify_event(connfd,EPOLLOUT|EPOLLET|EPOLLONESHOT);
    return NULL;
}

void *handleread(void *arg)        //处理读线程
{
    server *s=(server *)arg;
    while(1)
    {
        if(s->isReadEmpty())
        {
            sleep(1);
        }    
        else{
            int connfd=0;
            if(s->getReadFront(connfd))
                s->do_socket_read(connfd);
        }
    }
    return NULL;
}

void * processImage(void *arg)  //处理图片线程
{
    server *s=(server *)arg;
    while(1)
    {
        if(s->isProcessEmpty())
        {
            sleep(1);
        }    
        else{
            int connfd=0;
            if(s->getProcessFront(connfd))
            {
                int i;
                for(i=0;i<1000;i++)
                {
                    if(args[i].connfd==-1)
                        break;
                }
                args[i].s=s;
                args[i].connfd=connfd;
                //printf("connfd:%d\n",connfd);
                pool.add_task(k_means,(void *)&args[i]);          //k_means 放入线程池;
            }
        }
    }
    return NULL;
}

void *logThread(void *arg)  //处理日志线程
{
    server *s=(server *)arg;
    while(1)
    {
        sleep(2);
        s->logToDisk();
    }
    return NULL;
}

