#include "Log.h"

Log::Log()
{
    char filename[100];
    memset(filename,0,sizeof(filename));
    strcpy(filename,"./log/serverLog.txt");
    fp=fopen(filename,"w+");
    pthread_mutex_init(&lock,NULL);
}

Log::~Log()
{
    fclose(fp);
    pthread_mutex_destroy(&lock);
}

void Log::writeLog(char *info,bool debug)
{
    char tmp[500];
    memset(tmp,0,sizeof(tmp));
    strcpy(tmp,"TIME:");
    time_t timer;
    timer=time(NULL);
    struct tm *tblock;
    tblock=localtime(&timer);
    strcat(tmp,asctime(tblock));
    if(debug)
        strcat(tmp,"[DEBUG]");
    else
        strcat(tmp,"[SUCCESS]");
    strcat(tmp,info);
    strcat(tmp,"\n");
    pthread_mutex_lock(&lock);
    fwrite(tmp,strlen(tmp),1,fp);
    pthread_mutex_unlock(&lock);
}

void Log::writeToDisk(){
    pthread_mutex_lock(&lock);
    fflush(fp);
    fsync(fileno(fp));
    pthread_mutex_unlock(&lock);
}

