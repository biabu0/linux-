#include<stdlib.h>
#include<stdio.h>
#include<pthread.h> 
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<time.h>

#include "mytbf.h"


static struct mytbf_st* job[MYTBF_MAX];
static int inited = 0;
static pthread_mutex_t mut_job = PTHREAD_MUTEX_INITIALIZER;		//定义一个互斥量，保护job数组以独占的资源存在，在模块卸载的时候销毁
static pthread_t tid_alrm;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

struct mytbf_st
{
    int pos;
    int cps;
    int burst;
    int token;
	pthread_mutex_t mut;			//一个互斥量，每个线程中都有加上对于token,在调用fetchtoken的时候对token减，这样会产生冲突，锁住token，动态初始化，当结构体生成的时候才初始化
	pthread_cond_t cond;			//条件变量的数据结构
};

static void* thr_alrm(void *p)
{
    int i;
	struct timespec req;

	req.tv_sec = 1;
	req.tv_nsec = 0;
	while(1){
	pthread_mutex_lock(&mut_job);
    for(i = 0; i < MYTBF_MAX; i++)
    {
    
        if(job[i] != NULL){
			pthread_mutex_lock(&job[i]->mut);
            job[i]->token += job[i]->cps;
            if(job[i]->token > job[i]->burst)
                job[i]->token = job[i]->burst;
			pthread_cond_broadcast(&job[i]->cond);				//对token进行了加，唤醒等待
			pthread_mutex_unlock(&job[i]->mut);
        }
    }
	pthread_mutex_unlock(&mut_job);
	nanosleep(&req, NULL);				//一秒钟作一次
	}
}

static void module_unload(void)                 //关闭
{
	//unload由一个调用，不需要互斥量的使用
    int i;
    
	pthread_cancel(tid_alrm);							//取消指定的线程
	pthread_join(tid_alrm,NULL);						//
	
    for(i=0;i < MYTBF_MAX; i++){
		if(job[i] != NULL){
			mytbf_destroy(job[i]);
		}
	}
	pthread_mutex_destroy(&mut_job);				//销毁互斥量
    
}

static void module_load(void)
{
	int err;

	err = pthread_create(&tid_alrm, NULL, thr_alrm, NULL);
	if(err){
		fprintf(stderr,"pthread_create():%s\n", strerror(err));
		exit(1);
	}
    atexit(module_unload);                      //使用钩子函数，当函数异常终止时会调用钩子函数
}

static int get_free_pos_unlocked(void)			//函数名加入unlocked，提示用户函数为没有锁，如果使用，先加锁
{
    int i;
    for(i = 0; i < MYTBF_MAX; i++){
        if (job[i] == NULL)
            return i;
    }

    return -1;

}




mytbf_t *mytbf_init(int cps, int burst)
{
    struct mytbf_st *me;
    int pos;

	/*
	lock()
    if(!inited){					//初始化的时候调用,操作不原子，可能加载多个，可以使用互斥量，也可以使用pthread_once函数，保证之调用一次
        module_load();
        inited = 1;
    }
	unlock()
	*/
	pthread_once(&init_once, module_load);				//	动态模块的单次初始化函数

	me = malloc(sizeof(*me));
	if(me == NULL){
		return NULL;
	}

	me->token = 0;
	me->cps = cps;
	me->burst = burst;
	pthread_mutex_init(&me->mut, NULL);
	pthread_cond_init(&me->cond, NULL);

	pthread_mutex_lock(&mut_job);		//锁住，使用了全局变量job，避免冲突->锁
    pos = get_free_pos_unlocked();		//此处也应该锁住，避免出现多个线程同时调用，拿到同一个pos
										//调用函数也是跳转，需要注意
										//锁住的地方叫做临界区，临界区短则程序快
    if(pos < 0){
		pthread_mutex_unlock(&mut_job);
		free(me);
        return NULL;					//注意临界区的跳转语句，如果直接从这跳转，则死锁
	}
	/*缩短临界区，提出去
    me = malloc(sizeof(*me));
    if(me == NULL){
        return NULL; 
    }
    me->token = 0;
    me->cps = cps;
    me->burst = burst;
    */
	me->pos = pos;

    job[pos] = me;
	pthread_mutex_unlock(&mut_job);			//使用完job，解锁

    return me;
    
}


static int min(int a, int b)
{
    if(a < b)
        return a;
    else return b;
}

int mytbf_fetchtoken(mytbf_t *ptr, int size)
{
	//取size大小的令牌
    struct mytbf_st *me = ptr;
    int n;

    if(size < 0)
        return -EINVAL;            				 //参数非法
	
	pthread_mutex_lock(&me->mut);				//加锁
    while(me->token <= 0){
		pthread_cond_wait(&me->cond, &me->mut);			//对mut解锁，等待一个pthread的signal或者broadcast打断，得到后抢锁。相当于下面的代码
	/*//查询法，忙等，可以改成通知法
    	pthread_mutex_unlock(&me->mut);
		sched_yield();							//短时间出让调度器
		pthread_mutex_lock(&me->mut);
	*/
	}

    n = min(me->token, size);

    me->token -= n;
	pthread_mutex_unlock(&me->mut);				//操作完成，解锁

    return n;

}
int mytbf_returntoken(mytbf_t *ptr, int size)
{

    struct mytbf_st *me = ptr;

    if(size <=0)
        return -EINVAL;
    
	pthread_mutex_lock(&me->mut);
    me->token += size;
    if(me->token > me->burst)
        me->token = me->burst;
	pthread_cond_broadcast(&me->cond);				//并发的角度：多个人共用同一个令牌桶，a：取5个，b取十个；a取完后，b不满足取的条件则等待，当a取多了，return给令牌桶的时候，满足b的条件，此时应该给b一个信号中断等待
	pthread_mutex_unlock(&me->mut);

    return size;
}
                                        
                                       
int mytbf_destroy(mytbf_t *ptr)
{
    struct mytbf_st *me = ptr;
	
	pthread_mutex_lock(&mut_job);
    job[me->pos] = NULL;    
	pthread_mutex_unlock(&mut_job);
	pthread_mutex_destroy(&me->mut);				//销毁
    pthread_cond_destroy(&me->cond);
	free(ptr);

    return 0;
}




