#ifndef _EFUN_H
#define _EFUN_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <poll.h>
#include <assert.h>
#include <memory>
#include <string>
#include <pthread.h>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>
#define IPADDRESS "139.199.230.43"
#define PORT 9999
#define CONNECTNUM 1000 //listen监听数目
#define MAXLINE 1024
#define NAMELEN 100
using namespace std;
using namespace cv;

typedef void Sigfunc(int);
//int readAll(int sockfd,char * buff,int len);
int readn(int sockfd,char * buff,int len);
int writen(int sockfd,char *buff,int len);
//int writeAll(int sockfd,char *buff,int len);
Sigfunc *Signal(int signo,Sigfunc* func);

#endif
