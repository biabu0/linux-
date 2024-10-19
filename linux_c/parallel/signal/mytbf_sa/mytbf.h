#ifndef MYTBF_H__
#define MYTBF_H__

#define MYTBF_MAX   1024

typedef void mytbf_t;           //完成结构体的隐藏并增加可读性（相比于直接使用void）

mytbf_t *mytbf_init(int cps, int burst);


int mytbf_fetchtoken(mytbf_t *, int);
int mytbf_returntoken(mytbf_t *, int);


int mytbf_destroy(mytbf_t *);


#endif
