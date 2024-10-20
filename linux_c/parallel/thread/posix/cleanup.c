#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>




static void cleanup_func(void *p)
{
    puts(p);
}


static void *func(void *p)
{
    puts("Tread is eorking!");

    pthread_cleanup_push(cleanup_func, "cleanup:1");            //实现为宏，需要pop函数成对出现
                                                                //类似钩子函数,逆序调用，3，2，1
    pthread_cleanup_push(cleanup_func, "cleanup:2");
    pthread_cleanup_push(cleanup_func, "cleanup:3");
    
    puts("put over!");

    pthread_cleanup_pop(1);                         
    pthread_cleanup_pop(0);                                     //只弹出，不掉用
    pthread_cleanup_pop(1);


    pthread_exit(NULL);
}


int main()
{
    pthread_t tid;
    int err;
    puts("Begin!");

    err = pthread_create(&tid, NULL, func, NULL);
    if(err)
    {
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }
    
    pthread_join(tid, NULL);

    puts("End!");

    
    exit(0);
}
