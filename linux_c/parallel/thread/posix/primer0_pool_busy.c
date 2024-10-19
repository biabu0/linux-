#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<error.h>
#include<string.h>
#include<pthread.h>
#include<sched.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM  4

static int num = 0;
static pthread_mutex_t mut_num = PTHREAD_MUTEX_INITIALIZER;

static void *thr_prime(void *p);
int main()
{
    int i, err;

    pthread_t tid[THRNUM];

        
    for(i = 0; i < THRNUM; i++){
        err = pthread_create(tid+i, NULL, thr_prime, (void *)i);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }


    for(i = LEFT; i <= RIGHT; i++){
        pthread_mutex_lock(&mut_num);

        while(num != 0){
            pthread_mutex_unlock(&mut_num);         //先解锁
            
            sched_yield();                          //出让调度器给别的线程，不会造成调度颠簸，希望别的线程有机会拿到num或者改变num的状态
            pthread_mutex_lock(&mut_num);           //加锁
        }
        num = i;                                //当num = 0,任务被取走，给下一个任务
        pthread_mutex_unlock(&mut_num);
    }
    

    
    pthread_mutex_lock(&mut_num);
    while(num != 0){                                //确保最后一个任务已经被取走，如果不加该判断可能会导致刚解锁就又被main线程锁住，覆盖num
        pthread_mutex_unlock(&mut_num);
        sched_yield();
        pthread_mutex_lock(&mut_num);
    }

    num = -1;                                       //人物分配完，用于线程退出
    pthread_mutex_unlock(&mut_num);


    for(i = 0;i < THRNUM; i++)
        pthread_join(tid[i], NULL);
    

    pthread_mutex_destroy(&mut_num);
    
    exit(0);

}

static void *thr_prime(void *p)
{
    int mark;
    int i, j;
  
    while(1){

    pthread_mutex_lock(&mut_num);
    while(num == 0){
        pthread_mutex_unlock(&mut_num);
        sched_yield();
        pthread_mutex_lock(&mut_num);
    }
    
    if(num == -1){                  //结束
        pthread_mutex_unlock(&mut_num);
        break;                      //跳转去临界区外，如果不解锁会导致死锁
    }

    i = num;
    num = 0;

    pthread_mutex_unlock(&mut_num);
    mark = 1;
    for(j = 2; j < i/2; j++){
        if(i % j == 0){
            mark = 0;
            break;
        }
    }
    if(mark)
        printf("[%d]%d is a primer\n",(int)p, i);
    
    }
    pthread_exit(NULL);

}
