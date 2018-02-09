#include "server.h"
#include "ThreadPool.h"
#include "Singleton.h"
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

struct Arg{
    server *s;
    int connfd;
    Arg(server *ss,int k):s(ss),connfd(k){};
    Arg():s(NULL),connfd(-1){};
};

struct SigArg{
    server *s;
    sigset_t *set;
};

Arg args[1000];  //用于传递给线程池中的参数，防止在同一地址进行取值，造成取值不同的错误

//线程池有8个线程,线程池采用单例模式
ThreadPool &pool=Singleton<ThreadPool>::instance();

void * k_means(void *arg);//使用k-means得到图片主颜色
void *handleread(void *arg);        //处理读线程
void * processImage(void *arg);  //处理图片线程
void *logThread(void *arg);  //处理日志线程
void *handleSig(void *arg); //专门处理SIGALRM信号的线程,用来实现高效时间堆

int main()
{
    //主线程设置SIGALRM信号掩码,子线程继承信号屏蔽字
    sigset_t newset,oldset;
    sigemptyset(&newset);
    sigaddset(&newset,SIGALRM);
    int err;
    if((err=pthread_sigmask(SIG_BLOCK,&newset,&oldset))!=0)
    {
        perror("pthread_sigmask error\n");
        exit(1);
    }

    //server s;//服务器初始化,server采用单例模式
    server &s=Singleton<server>::instance();

    pthread_t pid[4];
    pthread_create(&pid[0],NULL,handleread,&s);
    pthread_create(&pid[1],NULL,processImage,&s);
    pthread_create(&pid[2],NULL,logThread,&s);
    SigArg sigarg;
    sigarg.s=&s;
    sigarg.set=&newset;
    pthread_create(&pid[3],NULL,handleSig,&sigarg);
    alarm(TIMESLOT);
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
            // else if(events[i].events & EPOLLOUT)
            // {
            //     s.do_socket_write(fd);
            // }
            else if(events[i].events & EPOLLIN)
            {
                s.doReadPush(fd);
            }
        }
    }

    for(int i=0;i<4;i++)
    {
        pthread_join(pid[i],NULL);
    }

    Singleton<ThreadPool>::deleteInstance();
    Singleton<server>::deleteInstance();

    exit(0);
}



//k_means实现函数
void * k_means(void *arg)
{
    Arg *a=(Arg *)arg;
    server *s=a->s;
    int connfd=a->connfd;
    if(s->findRecord(connfd)==false)  //如果已经被定时器删除则不处理
        return NULL;
    else {
        heap_timer *timer=s->getTimer(connfd);
        s->adjust_timer(timer);
    }
    printf("k_means connfd:%d\n",connfd);
    char outname[NAMELEN],imagename[NAMELEN];
    s->getDataFilename(imagename,connfd);
    a->connfd=-1;       //可被重新使用
    s->logWrite((char *)"k_means start",0);
    Mat small,image;
    int k;s->getDataK(k,connfd);
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
    memcpy(outname,imagename,strlen(imagename)-4);
    memcpy(outname+strlen(imagename)-4,"_mainColor.jpg",strlen("_mainColor.jpg"));
    outname[strlen(imagename)-4+strlen("_mainColor.jpg")]='\0';
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
    //printf("k_means_outname:%s\n",outname);
    imwrite(outname,small);
    //printf("k_means end\n");
    s->logWrite((char *)"k_means end",0);
    /*EPOLLOUT事件：
    //EPOLLOUT事件只有在连接时触发一次，表示可写，其他时候想要触发，那你要先准备好下面条件：
    //1.某次write，写满了发送缓冲区，返回错误码为EAGAIN。
    //2.对端读取了一些数据，又重新可写了，此时会触发EPOLLOUT。
    //简单地说：EPOLLOUT事件只有在不可写到可写的转变时刻，才会触发一次，所以叫边缘触发，这叫法没错的！
    */
    //s->epoll_add_event(connfd,EPOLLOUT|EPOLLIN|EPOLLET|EPOLLONESHOT); 
    s->do_socket_write(connfd);
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

void *handleSig(void *arg)
{
    SigArg *sigarg=(SigArg *)arg;
    server *s=sigarg->s;
    sigset_t *set=sigarg->set;
    int sig,n;
    while(true)
    {
        //调用sigwait等待信号
        n=sigwait(set,&sig);
        if(n!=0)
        {
            perror("sigwait error\n");
            pthread_exit((void *)1);
        }
        printf("start process tick\n");//printf不可重入，仅用于测试
        //定时处理任务，调用tick函数
        s->tick();
        //因为一次alarm调用只会引起一次SIGALRM信号，所以要重新定时
        alarm(TIMESLOT);
    }
}
