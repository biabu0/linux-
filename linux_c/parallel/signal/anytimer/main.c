#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include"anytimer.h"

static void f3(void *p)
{
    printf("f3():%s\n",p);
}
static void f2(void *p)
{
    printf("f2():%s\n",p);
}
static void f1(void *p)
{
    printf("f1():%s\n",p);
}

int main()
{
    int job1;

    puts("Begin!");
    
    job1 = at_addjob_repeat(5, f1, "aaa");
    if(job1 < 0){
        printf("........................");
        fprintf(stderr, "at_addjob():%s\n",strerror(-job1));
        exit(1);
    }
    
#if 0
    job2 = at_addjob(2, f2, "aaa");
    if(job2 < 0){
        fprintf(stderr, "at_addjob():%s\n",strerror(-job1));
        exit(1);
    }
    job3 = at_addjob(7, f3, "ccc");
    if(job3 < 0){
        fprintf(stderr, "at_addjob():%s\n",strerror(-job1));
        exit(1);
    }

#endif
    while(1)
    {
        write(1, ".", 1);
        sleep(1);
    }

    puts("End!");
    exit(1);
}
