#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<error.h>
#include<string.h>
#include<pthread.h>
#include"mysem.h"

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM  (RIGHT-LEFT+1)

#define N       4           //当前允许创建4个线程，信号量（使用互斥量和条件变量）

static mysem_t *sem;

static void *thr_prime(void *p);
int main()
{
    int i, err;

    pthread_t tid[THRNUM];

    sem = mysem_init(N);          //  设置资源总量
    if(sem == NULL){
        fprintf(stderr, "mysem_init() failed!\n");
        exit(1);
        
    }

        
    for(i = LEFT; i <= RIGHT; i++){
        mysem_sub(sem, 1);              //允许创建四个线程，每次减去1
        err = pthread_create(tid+(i-LEFT), NULL, thr_prime, (void *)i);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    for(i = LEFT;i <= RIGHT; i++)
        pthread_join(tid[i-LEFT], NULL);
    
    mysem_destroy(sem);

    exit(0);

}

static void *thr_prime(void *p)
{
    int mark = 1;
    int i, j;
    i = (int)p;

    for(j = 2; j < i/2; j++){
        if(i % j == 0){
            mark = 0;
            break;
        }
    }
    if(mark)
        printf("%d is a primer\n", i);
    
    sleep(5);                           //每一个进程要5s时间，方便使用ps查看进程关系

    mysem_add(sem, 1);                  //一个线程结束，加上资源量
    pthread_exit(NULL);

}
