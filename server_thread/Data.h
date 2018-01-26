#ifndef _DATA_H
#define _DATA_H

#include "efun.h"
class Data
{
public:
    int sockfd;
    long fileLen;
    int k;
    char imagename[NAMELEN];
    char filename[NAMELEN];
    Data():sockfd(0),fileLen(0),k(0){
        memset(imagename,0,NAMELEN);
        memset(filename,0,NAMELEN);
    }
};

#endif