#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/time.h>
#include<unistd.h>
#include<errno.h>

#include"anytimer.h"

enum
{
    STATE_RUNNING = 1,
    STATE_CANCELED,
    STATE_OVER
};

struct at_job_st
{
    int job_state;                      //job状态
    int sec;                            //多长时间工作，保持不变
    int time_remain;                    //时间的递减，原本等于sec；与sec一起可以处理sec出错的情况；可以用于周期性任务
    int repeat;                         //实现周期性任务    
    at_jobfunc_t *jobp;                  //函数
    void *arg;                          //函数传参
};

static struct at_job_st *job[JOB_MAX];
static struct sigaction alrm_sa_save;
static int get_free_pos(void)
{
    int i;
    for(i = 0; i < JOB_MAX; i++){
        if(job[i] == NULL)
            return i;
    }
    return -1;             
}

static int inited = 0;

static void alrm_action(int s, siginfo_t *infop, void *unused)
{
    int i;
    if(infop->si_code != SI_KERNEL)             //不是从kernel来则不想赢
        return ;

    for(i = 0; i < JOB_MAX; i++){
        
        //首先检查job[i]->job_state是否等于STATE_RUNNING。如果job[i]是NULL，则这个操作会立即导致段错误，因为程序试图访问一个空指针的成员。
        
        if(job[i] != NULL && job[i]->job_state == STATE_RUNNING){ 
            job[i]->time_remain --;
            if(job[i]->time_remain == 0){               //使用信号处理有所缺陷，jobp函数的运行可能时间较长
                job[i]->jobp(job[i]->arg);
                if(job[i]->repeat == 1)
                    job[i]->time_remain = job[i]->sec;
                else
                    job[i]->job_state = STATE_OVER;
            }

        }
    }
}

static void module_unload(void)
{
    struct itimerval itv;

    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, NULL);
    if(sigaction(SIGALRM, &alrm_sa_save, NULL) < 0)
    {
        perror("sigaction()");
        exit(1);
    }

}


static void module_load(void)
{
    struct sigaction sa;
    struct itimerval    itv;

    sa.sa_sigaction = alrm_action;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if(sigaction(SIGALRM, &sa, &alrm_sa_save) < 0)
    {
        perror("sigaction()");
        exit(1);                //
    }

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &itv, NULL) < 0)             //将时钟关掉，不需要保存旧
    {
        perror("setitimer()");
        exit(1);
    }

    atexit(module_unload);
}
int at_addjob(int sec, at_jobfunc_t *jobp, void *arg)
{
    int pos;
    struct at_job_st *me;
    
    if(sec < 0){
        return -EINVAL;
    }
    if(inited == 0){
        module_load();
        inited = 1;
    }
    
    pos = get_free_pos();
    if(pos < 0){
        return -ENOSPC;
    }

    me = malloc(sizeof(*me));
    if(me == NULL){
        return -ENOMEM;
    }
    me->job_state = STATE_RUNNING;
    me->sec = sec;
    me->time_remain = me->sec;
    me->jobp = jobp;
    me->arg = arg;
    me->repeat = 0;
    job[pos] = me;
    return pos;
        
}

int at_addjob_repeat(int sec, at_jobfunc_t *jobp, void *arg)
{
    int pos;
    struct at_job_st *me;

    if(sec < 0)
        return -EINVAL;
    if(!inited){
        module_load();
        inited = 1;
    }

    pos = get_free_pos();
    if(pos < 0)
        return -ENOSPC;

    me= malloc(sizeof(*me));
    if(me == NULL)
        return -ENOMEM;

    me->job_state = STATE_RUNNING;
    me->sec = sec;
    me->time_remain = me->sec;
    me->jobp = jobp;
    me->arg = arg;
    me->repeat = 1;
    job[pos] = me;
    return pos;

}

int at_canceljob(int id)
{
    if(id < 0 || id >= JOB_MAX || job[id] == NULL)
        return -EINVAL;
    if(job[id]->job_state == STATE_CANCELED)
        return -ECANCELED;
    if(job[id]->job_state == STATE_OVER)
        return -EBUSY;
    job[id]->job_state = STATE_CANCELED;
    return 0;

}

int at_waitjob(int id)
{
    if(id < 0 || id >= JOB_MAX || job[id] == NULL){
        return -EINVAL;
    }
    
    if(job[id]->repeat == 1)            //重复的收不回来
        return -EBUSY;

    while(job[id]->job_state == STATE_RUNNING)
        pause();

    if(job[id]->job_state == STATE_CANCELED || job[id]->job_state == STATE_OVER){
        free(job[id]);
        job[id] = NULL;
    }

    return 0;
    
}

/*
at_pausejob()
{


}

at_resumejob()
{


}
*/
