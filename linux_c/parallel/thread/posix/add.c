#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

/**竞争，故障
*20个线程，负责操作同一个文件，打开文件，读数据，取数据+1，覆盖写，关闭文件
*
*20个进程同时打开可以，但不能同时读写，使用互斥量
*
*
**/

#define THRNUM  20  
#define FNAME   "/tmp/out"
#define LINESIZE    1024


static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;         //静态分配的互斥量的初始化

static void *thr_add(void *p)
{
    FILE *fp;
    char linebuf[LINESIZE];

    fp = fopen(FNAME, "r+");
    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }
    
    pthread_mutex_lock(&mut);                   //从此刻锁住代码，读和写的操作,有一个线程运行，其他线程阻塞

    fgets(linebuf, LINESIZE,fp);                //文件类型指针后移
    fseek(fp, 0, SEEK_SET);                     //  文件类型指针定位到开始，进行覆盖写
    sleep(1);                                   //放大竞争和故障，等待时间长这样导致碰撞发生的概率高
    fprintf(fp, "%d\n", atoi(linebuf)+1);       //向文件中写+1
    fclose(fp);
    
    pthread_mutex_unlock(&mut);                 //释放互斥锁

    pthread_exit(NULL);
}

int main()
{
    int i, err;
    pthread_t tid[THRNUM];

    for(i = 0; i < THRNUM; i++){
        err = pthread_create(tid+i, NULL, thr_add, NULL);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    for(i = 0; i < THRNUM; i++){
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mut);

    
    exit(0);
}


