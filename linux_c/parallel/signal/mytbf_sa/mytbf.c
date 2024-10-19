#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<errno.h>
#include<unistd.h>
#include<sys/time.h>

#include "mytbf.h"

typedef void (*sighandler_t)(int);

static struct mytbf_st* job[MYTBF_MAX];
//static sighandler_t alrm_handler_save;          //保存旧的信号行为
static struct sigaction alrm_sa_save;
static int inited = 0;



struct mytbf_st
{
    int pos;
    int cps;
    int burst;
    int token;
};

static void alrm_action(int s, siginfo_t *infop, void *unused)
{
    int i;
	if(infop->si_code != SI_KERNEL) 				//程序中利用alarm和sigaction都是通过内核发出的
		return ;

    for(i = 0; i < MYTBF_MAX; i++)
    {
    
        if(job[i] != NULL){
            job[i]->token += job[i]->cps;
            if(job[i]->token > job[i]->burst)
                job[i]->token = job[i]->burst;
        }
    }
}

static void module_unload(void)                 //关闭
{
//  恢复SIGALRM功能，信号关掉，释放内容
    int i;
	struct itimerval itv;
    
	sigaction(SIGALRM, &alrm_sa_save, NULL);		//恢复旧行为

    itv.it_interval.tv_sec = 0;						//关闭时钟
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 0;
       
    if(setitimer(ITIMER_REAL, &itv, NULL) < 0)                      //时钟类型
    {
		perror("setitimer()");
        exit(1);
    }

	

    for(i=0;i < MYTBF_MAX; i++)
        free(job[i]);
    
}

static void module_load(void)
{
	
	struct sigaction sa;
	struct itimerval itv;

	sa.sa_sigaction = alrm_action;							//三个参数的形式可以保存信号的来源
	sigemptyset(&sa.sa_mask);
	sa.sa_flags =SA_SIGINFO;								//特殊要求，使用三个参数的信号处理程序


	if(sigaction(SIGALRM, &sa, &alrm_sa_save) < 0)
	{
		perror("sigaction()");
		exit(1);
	}
	
	itv.it_interval.tv_sec = 1;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;

	if(setitimer(ITIMER_REAL, &itv, NULL) < 0)						//时钟类型
	{
		perror("setitimer()");
		exit(1);
	}
	

    atexit(module_unload);                      //使用钩子函数，当函数异常终止时会调用钩子函数
}

static int get_free_pos(void)
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

    if(!inited){
        module_load();
        inited = 1;
    }


    pos = get_free_pos();
    if(pos < 0)
        return NULL;

    me = malloc(sizeof(*me));
    if(me == NULL){
        return NULL;
    }
    me->token = 0;
    me->cps = cps;
    me->burst = burst;
    me->pos = pos;

    job[pos] = me;

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
    struct mytbf_st *me = ptr;
    int n;

    if(size < 0)
        return -EINVAL;             //参数非法

    while(me->token <= 0)
        pause();

    n = min(me->token, size);

    me->token -= n;

    return n;

}
int mytbf_returntoken(mytbf_t *ptr, int size)
{

    struct mytbf_st *me = ptr;

    if(size <=0)
        return -EINVAL;
    
    me->token += size;
    if(me->token > me->burst)
        me->token = me->burst;

    return size;
}
                                        
                                       
int mytbf_destroy(mytbf_t *ptr)
{
    struct mytbf_st *me = ptr;

    job[me->pos] = NULL;    
    free(ptr);

    return 0;
}













