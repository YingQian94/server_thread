#include <unistd.h> 
#include <sys/types.h> 
#include <stdio.h> 
#include <stdlib.h> 
int main(int argc,char * argv[]) 
{ 
    int count =10,i; 
    for(i=0;i<count;i++) 
    { 
        pid_t pid = fork(); 
        if(pid == 0) 
        { 
            for(int j=0;j<20;++j)
                execl("/home/ubuntu/server_vThread/server_thread/client","./client", "4",
                "/home/ubuntu/timg3.jpg","/home/ubuntu/clientImg/","2",
                "/home/ubuntu/timg4.jpg","/home/ubuntu/clientImg/","2",
                "/home/ubuntu/timg5.jpg","/home/ubuntu/clientImg/","2",
                "/home/ubuntu/timg2.jpg","/home/ubuntu/clientImg/","2",(char *)0); 
            return(0); 
        } 
    } 
    return(0); 
}   
