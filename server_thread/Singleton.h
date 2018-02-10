#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <pthread.h>

template<typename T>
class Singleton{
private:
    Singleton();
    ~Singleton();
    Singleton(const Singleton&)=delete;
    Singleton & operator=(const Singleton&)=delete;
    static void init()
    {
        value=new T();
    }
public:
    static T& instance()
    {
        pthread_once(&once,&Singleton::init);
        return *value;
    }
    static void deleteInstance()
    {
        delete value;
        value=NULL;
    }
private:
    static pthread_once_t once;
    static T* value;
};

template<typename T>
pthread_once_t Singleton<T>::once=PTHREAD_ONCE_INIT;

template<typename T>
T *Singleton<T>::value=NULL;

#endif