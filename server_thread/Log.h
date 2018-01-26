#ifndef _LOG_H
#define _LOG_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <mutex>
class Log{
    FILE *fp;
    pthread_mutex_t lock;
    Log(const Log&);         //防止资源出现复制错误
    Log& operator=(const Log&);
public:
    Log();
    void writeLog(char *info,bool debug);
    void writeToDisk();
    ~Log();
};

#endif