#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<error.h>
#include<string.h>
#include<pthread.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM  (RIGHT-LEFT+1)      //资源数一定要有一个上限，使用这样的会创建201个线程，但如果是两万个则会出问题，有一个资源上限，如何使特定的线程数进行运行，3，4，5，6个这样的呢？



struct thr_arg_st
{
    int n;
};


static void *thr_prime(void *p);
int main()
{
    int i, err;
    struct thr_arg_st *p;
    void *ptr;

    pthread_t tid[THRNUM];

        
    for(i = LEFT; i <= RIGHT; i++){
    
        p = malloc(sizeof(*p));
        if(p == NULL){
            perror("malloc()");
            exit(1);
        }

        p->n = i;
        
        err = pthread_create(tid+(i-LEFT), NULL, thr_prime, p);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    for(i = LEFT;i <= RIGHT; i++){
        pthread_join(tid[i-LEFT], &ptr);        //接受从exit传回的数据p
        free(ptr);
    }

    exit(0);

}

static void *thr_prime(void *p)
{
    int mark = 1;
    int i, j;
    i = ((struct thr_arg_st *)p)->n;
    
    //free(p);              //要将free与malloc放在同一个模块中，使用pthread_exit()返回p

    for(j = 2; j < i/2; j++){
        if(i % j == 0){
            mark = 0;
            break;
        }
    }
    if(mark)
        printf("%d is a primer\n", i);
    
    pthread_exit(p);

}
