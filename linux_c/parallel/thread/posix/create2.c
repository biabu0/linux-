#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

static void * func(void *p)
{
    while(1){
        pause();
    }
    pthread_exit(NULL);                 //对线程的返回值不感兴趣

}

int main()
{
    pthread_t tid;
    int i, err;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr,1024*1024);             //设置属性，栈的大小为1MB

    for(i = 0; ; i++){
        err = pthread_create(&tid, &attr, func, NULL);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            break;
        }
    }


    printf("%d\n", i);                  //栈的大小为8MB的时候，不设置属性的时候是20576
    
    pthread_attr_destroy(&attr);
    exit(0);
}

