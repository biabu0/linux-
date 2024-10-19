//使用条件变量和互斥量完成一个能够记次数的有资源上限的资源共享
#ifndef MYSEM_H__
#define MYSEM_H__





typedef void mysem_t;

mysem_t *mysem_init(int initval);


int mysem_add(mysem_t *, int );
int mysem_sub(mysem_t *, int );


int mysem_destroy(mysem_t *);










#endif

