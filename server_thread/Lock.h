#ifndef _LOCK_H
#define _LOCK_H

#include <memory>
#include <pthread.h>
using namespace std;

/*****************************************/
/*RAII 使用智能指针来防止出现mutex对象的复制操作引发的问题。
当智能指针引用计数为0时默认删除其所指物，需要指定删除器
来处理引用计数为0时，unlock mutex。
Lock不需要声明析构函数，而是利用编译器生成的析构函数，
生成的析构函数会自动调用其non-static成员变量的析构函数，mutexPtr
的析构函数会在mutex的引用计数为0时自动调用shared_ptr的删除器。
*/
/*****************************************/
// class Lock{
// public:
//     explicit Lock(pthread_mutex_t *mx)
//     :mutexPtr(mx,pthread_mutex_unlock)
//     {
//         pthread_mutex_lock(mutexPtr.get());
//     }
// private:
//     shared_ptr<pthread_mutex_t> mutexPtr;
// };

class Lock{
public:
    Lock(pthread_mutex_t *mx):mutex(mx)
    {
        pthread_mutex_lock(mutex);
    }
    ~Lock()
    {
        pthread_mutex_unlock(mutex);
        mutex=NULL;
    }
    Lock(const Lock&);
    Lock& operator=(const Lock&);
private:
    pthread_mutex_t *mutex;
};

#endif
