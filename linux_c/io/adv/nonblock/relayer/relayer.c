#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

#include"relayer.h"
#define BUFSIZE 1024

enum
{
    STATE_R = 1,            //枚举使用逗号
    STATE_W,
    STATE_Ex,
    STATE_T
};


struct rel_fsm_st                           //有限状态机的结构体
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];                  //读出来的数据放到缓冲区，用于写
    int len;                            //存读出来的数据大小
    int pos;                            //从buf中的哪个位置开始写（坚持写够）
    char *errstr;                       //出错的来源
	int64_t count;						//记录交换的数据个数即count12,count21
};


struct rel_job_st
{
	int job_state;
	int fd1;
	int fd2;
	int fd1_save;
	int fd2_save;
	struct rel_fsm_st fsm12, fsm21;
	//struct timerval start, end;

};

static struct rel_job_st* rel_job[REL_JOBMAX];				//job工作数组
static pthread_mutex_t mut_rel_job = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static pthread_t tid_relayer;


static void fsm_driver(struct rel_fsm_st *fsm)
{
    int ret;

    switch(fsm->state){
        case STATE_R:
            fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
            if(fsm->len == 0)
                fsm->state = STATE_T;
            else if(fsm->len < 0){
                if(errno == EAGAIN)
                    fsm->state = STATE_R;
                else{
                    fsm->errstr = "read()";
                    fsm->state = STATE_Ex;
                }
            }
            else{
                fsm->pos = 0;
                fsm->state = STATE_W;
            }
            break;
        case STATE_W:
            ret = write(fsm->dfd, fsm->buf+fsm->pos, fsm->len);
            if(ret < 0)
            {
                if(errno == EAGAIN)
                    fsm->state = STATE_W;
                else{
                    fsm->errstr = "write()";
                    fsm->state = STATE_Ex;
                }
            }
            else
            {
                fsm->pos += ret;
                fsm->len -= ret;
                if(fsm->len == 0)
                    fsm->state = STATE_R;
                 else
                    fsm->state = STATE_W;
            }
            break;

        case STATE_Ex:
            perror(fsm->errstr);                    //出错报错
            fsm->state = STATE_T;
            break;

        case STATE_T:
            /*do sth*/
            break;
        default:
            /*do sth*/
            abort();                                //导致进程异常终止
            break;
    }

}


static void* thr_relayer(void *p)
{
	int i;

	while(1){
		
		pthread_mutex_lock(&mut_rel_job);
		for(i = 0; i < REL_JOBMAX; i++){
			if(rel_job[i] != NULL){
				if(rel_job[i]->job_state == STATE_RUNNING){
					fsm_driver(&rel_job[i]->fsm12);
					fsm_driver(&rel_job[i]->fsm21);
					if(rel_job[i]->fsm12.state == STATE_T && rel_job[i]->fsm21.state == STATE_T)
						rel_job[i]->job_state = STATE_OVER;	
				}
			}
		}
		pthread_mutex_unlock(&mut_rel_job);
	
	}
}


static void module_unload(void)
{
	int i;
	pthread_cancel(tid_relayer);
	pthread_join(tid_relayer, NULL);
	
	//for();							//销毁job数组
	
	pthread_mutex_destroy(&mut_rel_job);
}


static void module_load(void)					//创建一个线程，永远的推状态机
{
	
	int err;

	err = pthread_create(&tid_relayer, NULL, thr_relayer, NULL);
	if(err){
		fprintf(stderr, "pthread_create():%s\n", strerror(err));
		exit(1);
	}
	
	atexit(module_unload);
}



static int get_free_pos_unlocked()
{
	int i;
	for(i = 0; i < REL_JOBMAX; i++)
	{
		if(rel_job[i] == NULL)
			return i;
	}
	return -1;
}


int rel_addjob(int fd1, int fd2)
{
	struct rel_job_st *me;
	int pos;
	
	pthread_once(&init_once, module_load);					//只需要第一个来条用进程进行推动状态机，使用动态模块的单次初始化函数

	me = malloc(sizeof(*me));
	if(me == NULL)
		return -ENOMEM;
	
	me->fd1 = fd1;
	me->fd2 = fd2;
	me->job_state = STATE_RUNNING;
	
	//获取时间戳；任务cancel或者over再次获取
	me->fd1_save = fcntl(me->fd1, F_GETFL);				//不能确保当前的fd是非阻塞的
	fcntl(me->fd1, F_SETFL, me->fd1_save|O_NONBLOCK);
	me->fd2_save = fcntl(me->fd2, F_GETFL);
	fcntl(me->fd2, F_SETFL, me->fd2_save|O_NONBLOCK);

	me->fsm12.sfd = fd1;
	me->fsm12.dfd = fd2;
	me->fsm12.state = STATE_R;
	
	me->fsm21.sfd = fd2;
	me->fsm21.dfd = fd1;
	me->fsm21.state = STATE_R;


	pthread_mutex_lock(&mut_rel_job);
	pos = get_free_pos_unlocked();						//不支持多线程并发，通过名字体现出
	if(pos < 0){
		pthread_mutex_unlock(&mut_rel_job);				//临界区中的跳转，解锁，恢复现场
		fcntl(me->fd1, F_SETFL, me->fd1_save);
		fcntl(me->fd2, F_SETFL, me->fd2_save);
		free(me);
		return -ENOSPC;
	}

	rel_job[pos] = me;
	pthread_mutex_unlock(&mut_rel_job);

	return pos;

}
#if 0
int rel_canceljob()
{

}

int rel_waitjob()
{

}

int rel_statjob()
{

}
#endif














