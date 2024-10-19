#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include"mypipe.h"

struct mypipe_st
{
    int head;
    int tail;
    char data[PIPESIZE];
    int datasize;                       //管道中有多少字节数
    int count_r;
    int count_w;
    pthread_mutex_t mut;                //不能同时操作head和tail，使用互斥量独占
    pthread_cond_t cond;                //条件变量，用于通知管道中有没有数据
};



mypipe_t *mypipe_init(void)
{
    struct mypipe_st *me;

    me = malloc(sizeof(*me));
    if(me == NULL)
        return NULL;                        //不要轻易在.c（库）文件中报可有可无的错，容易根用户报错冲突
    me->head = 0;
    me->tail = 0;
    me->datasize = 0;
    me->count_r = 0;
    me->count_w = 0;
    pthread_mutex_init(&me->mut, NULL);
    pthread_cond_init(&me->cond, NULL);

    return me;
}

static int next(int head)
{
    if(head == PIPESIZE -1) 
        return 0;
    return head + 1;
}


static int mypipe_register(mypipe_t *ptr, int opmap)
{
    struct mypipe_st *me = ptr;
    
    if(!(opmap & MYPIPE_READ) || !(OPMAP & MYPIPE_WRITE)){
        return -1;                                          //不是读也不是写 ，参数非法
    }

    pthread_mutex_lock(&me->mut);                           //会与unregister产生冲突，加锁
    if(opmap & MYPIPE_READ)
        me->count_r++;
    if(opmap & MYPIPE_WRITE)
        me->count_w++;

    pthread_cond_broadcast(&me->cond);                      //惊群；
    if(me->count_r <= 0 || me->count_w <= 0)
        pthread_cond_wait(&me->cond, &me->mut);                 //管道中至少要有一个读一个写

    pthread_mutex_unlock(&me->mut);

   return 0;

}

static int mypipe_unregister(mypipe_t *ptr, int opmap)
{
    struct mypipe_st *me = ptr;

    if(!(opmap & MYPIPE_READ) || !(OPMAP & MYPIPE_WRITE)){
        return -1;                                          //不是读也不是写 ，参数非法
    }

    pthread_mutex_lock(&me->mut);

    if(opmap & MYPIPE_READ)
        me->count_r--;
    if(opmap & MYPIPE_WRITE)
        me->count_w--;

    pthread_cond_broadcast(&me->cond);                      //惊群；
//    if(me->count_r <= 0 || me->count_w <= 0)
//        pthread_cond_wait(&me->cond, &me->mut);                 //管道中至少要有一个读一个写
    
    pthread_mutex_unlock(&me->mut);
    return 0;
    
}


static int mypipe_readbyte_unlocked(struct mypipe_st *me, char *datap)
{

    if(me->datasize <= 0)
        return -1;
    datap = me->data[me->head];
    me->head = next(me->head);
    me->datasize--;

    return 0;
    
}


int mypipe_read(mypipe_t *ptr, void *buf, size_t count)
{
    struct mypipe_st *me = ptr;

    pthread_mutex_lock(&me->mut);
    
    while(me->datasize <= 0 && me->count_w > 0)                 //没有数据，但是要有写的才等待，不然等待无意义
        pthread_cond_wait(&me->cond, &me->mut);                 //两个变化
    
    if(me->datasize <= 0 && me->count_w <= 0){                      //管道为空并且没有写的则exit()
        pthread_mutex_unlock(&me->mut);
        return 0;
    }

    for(i = 0; i < count; i++){
        if(mypipe_readbyte_unlocked(me, buf) != 0)
            break;                                              //临界区内部
    }
    
    pthread_cond_broadcast(&me->cond);                          //通知write有空间可以写
    pthread_mutex_unlock(&me->mut);

    return i;
}

int mypipe_write(mypipe_t *ptr, void *buf, size_t count)
{
    struct mypipe_st *me = ptr;
    
    pthread_mutex_lock(&me->mut);

    while(me->datasize == PIPESIZE && me->count_r > 0)
        pthread_cond_wait(&me->cond, &me->mut);

    if(me->datasize == PIPESIZE && me->count_r <= 0){
        pthread_mutex_unlock(&me->mut);
        return 0;
    }
    
    for(i= 0; i < count; i++){
        if(mypipe_writebyte(me, buf) != 0);
            break;
    }

    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);
    return i;
}

int mypipe_destroy(mypipe_t *ptr)
{
    struct mypipe_st *me = ptr;                          

    pthread_mutex_destroy(&me->mut);
    pthread_cond_destroy(&me->cond);

    free(ptr);
    return 0;
}




















