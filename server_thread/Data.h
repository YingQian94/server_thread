#ifndef _DATA_H
#define _DATA_H

#include "efun.h"
#include "Lock.h"
#include "Timer.h"
#include <memory>
#include <map>
#include <thread>

class heap_timer;
class Data
{
public:
    int sockfd;                             
    long fileLen;
    int k;                                  
    char imagename[NAMELEN];
    char filename[NAMELEN];
    heap_timer *timer=NULL;
    Data():sockfd(0),fileLen(0),k(0){
        memset(imagename,0,NAMELEN);
        memset(filename,0,NAMELEN);
    }
};

// /*使用shared_ptr 和 weak_ptr控制data对象的对象池*/
// class DataFactory{          
// public:
//     shared_ptr<Data> get(const int &key);
//     DataFactory()
//     {
//         pthread_mutex_init(&mutex,NULL);
//     }
//     ~DataFactory()
//     {
//         pthread_mutex_destroy(&mutex);
//     }
// private:
//     pthread_mutex_t mutex;
//     /*注意map中不能使用shared_ptr,因为shared_ptr是强引用
//     将导致对象引用计数一直大于0，无法被销毁*/
//     map<int,weak_ptr<Data>> serverData;
//     void deleteData(Data *data);
// };

#endif