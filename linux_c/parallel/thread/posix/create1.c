#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>

static void * func(void *p)
{
    puts("Pthread is working!");
    pthread_exit(NULL);                      //可以做线程栈的清理
//    return NULL;
}


int main()
{   
    pthread_t tid;
    int err;

    puts("Begin!");


    err = pthread_create(&tid, NULL, func, NULL);
    if(err){
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }

    pthread_join(tid, NULL);                //等待对线程的收尸，使用此函数可以保证新创建的线程执行，否则，由于调度器的调度策略不同，新创建的线程可能还没有执行完，main线程就已经结束

    puts("End!");



    exit(0);
}
