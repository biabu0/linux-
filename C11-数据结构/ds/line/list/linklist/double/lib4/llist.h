#ifndef LLIST_H__
#define LLIST_H__

#define LLIST_FORWARD  1
#define LLIST_BACKWARD 2

// !!! 实现了隐藏 LLIST 的实现方法
typedef void LLIST;

typedef void llist_op(const void *);
typedef int  llist_cmp(const void *, const void *);

LLIST *llist_create(int initsize);

int llist_insert(LLIST *, const void *data, int mode);

void *llist_find(LLIST *, const void *key, llist_cmp *);

int llist_delete(LLIST *, const void *key, llist_cmp *);

int llist_fetch(LLIST *, const void *key, llist_cmp *, void *data);

void llist_travel(LLIST *, llist_op *);

void llist_destroy(LLIST *);

#endif
