#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define FNAME  "/tmp/out"

int main()
{
    int fd;

    fd = open(FNAME,O_WRONLY|O_CREAT|O_TRUNC,0600);
    if(fd < 0)
    {
        perror("open()");
        exit(1);
    }
    //下面的两步操作不原子，如果是多进程多线程并发，关闭1后可能另一个进程立马打开了一个文件，占用了1，此时dup(fd)则将复制到fd = 4的位置,使用dup2代替
    //close(1);//关闭标准输出
    //dup(fd);//将fd复制到可用的最小的文件描述符上，即，将fd复制到1上；

    
    dup2(fd, 1);

    if(fd != 1)   // 如果dup2中fd = 1，不会操作关闭close(1);
        close(fd);//关闭fd，此时文件并没有关闭，有个打开计数器，现在为2-1


/****************************************************/
    puts("hello!");
    exit(0);
}
