#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>


int main()
{
    pid_t pid;
    printf("[%d]: Begin!\n", getpid());//只有在标准输出的时候刷新缓冲区，其他是全缓冲刷新模式时会导致该缓冲区的内容同样复制到子进程中。
    
    fflush(NULL);/******一定要在fork之前刷新流：不刷新流，则存储在缓冲区，在进行fork时会将父进程的缓冲区复制一份给子进程，此时，子进程会有父进程fork之前的内容*******/
        
    pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }
    if(pid == 0){
        printf("[%d]: Child is working!\n", getpid());
    }
    else
        printf("[%d]: Parent is working!\n", getpid());


    printf("[%d]: End!\n", getpid());
    
    //getchar();使用ps axf命令可以查看父子进程之间的关系

    exit(0);
}
