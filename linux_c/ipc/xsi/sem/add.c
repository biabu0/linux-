#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<errno.h>


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

static int semid;


static void P(void)
{
    struct sembuf op;                   //只有一个
    
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;

    while(semop(semid, &op, 1) < 0){
        if(errno != EINTR || errno != EAGAIN){
            perror("semop()");
            exit(1);
        }
    }
}

static void V(void)
{
    struct sembuf op;
    
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;

    if(semop(semid, &op, 1) < 0){
        perror("semop()");
        exit(1);
    }
}

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
    
//    fd = fileno(fp);                            //FILE流中一定有一个文件描述符，使用fileno获取
//    lockf(fd, F_LOCK, 0);                       //0表示有多长锁多长
    
    P();
    fgets(linebuf, LINESIZE,fp);                //文件类型指针后移
    fseek(fp, 0, SEEK_SET);                     //  文件类型指针定位到开始，进行覆盖写
    sleep(1);                                   //放大竞争和故障，等待时间长这样导致碰撞发生的概率高
    fprintf(fp, "%d\n", atoi(linebuf)+1);       //向文件中写+1，行缓冲模式
    fflush(fp);                                 //刷新

    V();

    //fclose(fp);                               //这里可能会出现文件意外解锁问题
//    lockf(fd, F_ULOCK, 0);                      //解锁

    fclose(fp);



}

int main()
{
    int i, pid;
    
    semid = semget(IPC_PRIVATE, 1, 0600);                   //父子进程之间通信，不关心key，古不需要使用key, 只有一个成员，也就是信号量数组中的数为1，相当于lock
    if(semid < 0){
        perror("semget()");
        exit(1);
    }

    if(semctl(semid, 0, SETVAL, 1) < 0)            //下标为0的成员设置资源量为1
    {
        perror("semctl()");
        exit(1);
    }

    


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



    semctl(semid, 0, IPC_RMID);                     //销毁
    exit(0);
}


