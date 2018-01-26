#ifndef _CLIENT_H
#define _CLIENT_H

#include "efun.h"
#define MAXFILES 20

struct file{
    char *f_name;
    char *storepath;
    int f_fd;
    int k;
    int f_flags;
}file[MAXFILES];

#define F_CONNECTING 1
#define F_READING 2
#define F_DONE 4

class client
{
public:
    client();
    int maxfd;
    fd_set rset,wset;
    void startConnect(struct file* fptr);
    void sendImage(struct file* fptr);
    void receiveImage(struct file *fptr); 
};

#endif
