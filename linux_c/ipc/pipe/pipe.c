#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define  BUFSIZE  1024
/*  从网络中收取数据给子进程，子进程通过解码器播放  */
int main()
{
    int pipefd[2];
    pid_t pid;
    char buf[BUFSIZE];
    int len;

    if(pipe(pipefd) < 0){
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }
    if(pid == 0){                   //child read
        close(pipefd[1]);
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        fd = open("/dev/null", O_RDWR);
        dup2(fd, 1);                //不需要
        dup2(fd, 2);
        execl("/usr/bin/mpg123", "mpg123", "-", NULL);
        perror("execl()");
        exet(1);

    }
    else                            //parent write
    {
        close(pipefd[0]);
        //父进程从网上收取数据，向管道中写

        //待完善
        close(pipefd[1]);
        wait(NULL);                 //等待收尸
        exit(0);
    }

}
