#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


/**竞争，故障
*20个进程，负责操作同一个文件，打开文件，读数据，取数据+1，覆盖写，关闭文件
*
*20个进程同时打开可以，但不能同时读写，使用互斥量
*
*
**/

#define PROCNUM  20  
#define FNAME   "/tmp/out"
#define LINESIZE    1024




static void func_add(void)
{
    FILE *fp;
    char linebuf[LINESIZE];
    int fd;

    fp = fopen(FNAME, "r+");
    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }
    
    fd = fileno(fp);                            //FILE流中一定有一个文件描述符，使用fileno获取
    lockf(fd, F_LOCK, 0);                       //0表示有多长锁多长
    fgets(linebuf, LINESIZE,fp);                //文件类型指针后移
    fseek(fp, 0, SEEK_SET);                     //  文件类型指针定位到开始，进行覆盖写
    sleep(1);                                   //放大竞争和故障，等待时间长这样导致碰撞发生的概率高
    fprintf(fp, "%d\n", atoi(linebuf)+1);       //向文件中写+1，行缓冲模式
    fflush(fp);                                 //刷新
    //fclose(fp);                               //这里可能会出现文件意外解锁问题
    lockf(fd, F_ULOCK, 0);                      //解锁

    fclose(fp);



}

int main()
{
    int i, pid;


    for(i = 0; i < PROCNUM; i++){

        pid = fork();
        if(pid < 0){
            perror("fork()");
            exit(1);
        }

        if(pid == 0){
            func_add();
            exit(0);
        }
    }

    for(i = 0; i < PROCNUM; i++){
        wait(NULL);
    }



    
    exit(0);
}


