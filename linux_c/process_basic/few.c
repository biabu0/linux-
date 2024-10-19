#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
/*
* date +%s
*/

int main()
{
    pid_t pid;

    puts("Begin!");     //puts自动换行
    fflush(NULL);       //exec和fork之前需要刷新缓冲区，不然全缓冲才刷新，此时调用exec产生了新的进程映像替换了旧的进程映像,导致execl之前的内容出现问题。
    
    pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }
    if(pid == 0){
    
        execl("/bin/date", "date", "+%s", NULL);{
            perror("execl()");
            exit(1);
        }
    }

    wait(NULL);
    
    puts("End!");
    
    exit(0);
}


