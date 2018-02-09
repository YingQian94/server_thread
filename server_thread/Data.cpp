#include "Data.h"
#include <functional>
using namespace std::placeholders;

shared_ptr<Data> DataFactory::get(const int &key)
{
    shared_ptr<Data> pData;
    Lock l(&(this->mutex));
    weak_ptr<Data> &wData=serverData[key]; //如果key不存在会默认构造一个weak_ptr
    /*将weak_ptr转为shared_ptr,弱引用转为强引用，如果对象还活着，那么他可以提升为有效的shared_ptr,
    如果对象已经死了，提升会失败，返回一个空的shared_ptr*/
    pData=wData.lock();                    
    if(!pData)
    {
        /*pData 指向new Data(key)指针，将会调用bind(&DataFactory::deleteData,this,_1)释放新指针*/
        pData.reset(new Data(key),bind(&DataFactory::deleteData,this,_1));
        wData=pData;
    }
    return pData;
}

/*假设DataFactory的生存期比所有Data的生存期要久*/
void DataFactory::deleteData(Data *data)
{
    if(data)
    {
        Lock l(&(this->mutex));
        serverData.erase(stock->key());
    }
    delete data;
}