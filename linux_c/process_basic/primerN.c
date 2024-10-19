#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define LEFT    300000000
#define RIGHT   300000200
#define N       3

int main()
{
    int i, j, n;
    int mark;
    pid_t pid;
    

    for(n = 0; n < N; n++)
    {
        pid = fork();
        if(pid < 0){
            perror("fork()");
            //出错后，fork出去的wait回来，释放资源
            exit(1);
        }
        if(pid == 0){
        
            for(i = LEFT+n; i <= RIGHT; i+= N){
                mark = 1;
                for(j = 2; j < i/2; j++){
                    if(i % j == 0){
                        mark = 0;
                        break;
                    }
                }
                if(mark)
                   printf("[%d]: %d is a primer\n",n, i);
         //   sleep(1000);    //让父进程先结束，子进程变成孤儿态，交由init处理，exit(0)结束后释放资源。
    
            }
            exit(0);
        }
    }
    
    //int st;
    for(n = 0; n <= N; n++){
        //wait(&st);
        wait(NULL);
    }

/**
父进程状态S+，可中断的睡眠态，子进程是Z+，僵尸态；谁打开谁关闭，谁申请谁释放；需要将僵尸态收尸，使用进程的消亡和资源的释放；当sleep(1000)结束，父进程结束，子进程成为孤儿态，交由init接管，init进行收尸，释放资源。主要释放pid（有上限），是否关心进程的结束。
**/
//    sleep(1000);

    exit(0);
}
