#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

#define THRNUM      4


static pthread_mutex_t mut[THRNUM];

/**
*使用四个线程，向终端输入abcd
*
**/
static int next(int n)
{
    if(n + 1 == THRNUM)
        return 0;
    return n + 1;
}

static void* thr_func(void* p)
{
    int n = (int)p;
    char c = 'a' + n;

    while(1){
    //构建锁链
        pthread_mutex_lock(mut+n);          //锁自己等待下一次打开
        write(1, &c, 1);
        pthread_mutex_unlock(mut+next(n));
    }
    pthread_exit(NULL);             //执行不到
}


int main()
{
    int i, err;
    pthread_t tid[THRNUM];
    
    for(i = 0; i < THRNUM; i++){
        
        pthread_mutex_init(mut+i, NULL);
        pthread_mutex_lock(mut+i);

        err = pthread_create(tid+i, NULL, thr_func, (void *)i);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    pthread_mutex_unlock(mut+0);            //解开a的锁


    alarm(5);                       //5秒后sigalrm信号被发送到进程时，进程默认执行sigalrm信号处理，终止进程。
    
//    for(i = 0; i < THRNUM; i++){
//        phread_mutex_destroy(mut+i);
//    }


    for(i = 0; i < THRNUM; i++){
        pthread_join(tid[i], NULL);
    }



    exit(0);
}
