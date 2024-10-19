#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>

#define MEMSIZE  1024

int main()
{
    char *ptr;    
    pid_t pid;

    ptr = mmap(NULL, MEMSIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);           //子进程写，父进程读，子进程you父进程fork出来，在此时jiu应该可读可写，没有依赖于任何的文件，使用匿名映射


    pid = fork();
    if(pid < 0){
        perror("fork()");
        munmap(ptr, MEMSIZE);                           //这里已经映射成功
        exit(1);
    }
    
    if(pid == 0){                                       //child write
        strcpy(ptr, "hello!");
        munmap(ptr, MEMSIZE);                           //解的时进程自己的虚拟内存中与这块物理内存空间相关联的映射
        exit(1);
    }
    else{                                               //parent read
        wait(NULL);                                     //等待子进程结束，这个时候肯定写完了；另外防止子进程变成jiangshi进程
        puts(ptr);
        munmap(ptr, MEMSIZE);
        exit(1);
    }

}
