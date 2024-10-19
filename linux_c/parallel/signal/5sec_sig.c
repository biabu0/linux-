#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


static volatile int loop = 1;           //用volatile关键字可以告诉编译器，这个变量的值可能会随时改变，因此编译器在每次访问这个变量时都必须直接从内存中读取其值，而不能使用缓存中的值。

static void alarm_handler(int s)
{
    loop = 0;
}


int main()
{
    
    int64_t count;

    signal(SIGALRM, alarm_handler); //signal要在alarm之前
    alarm(5);                       //忽略或者不捕捉此信号会终止调用该alarm函数的进程

    while(loop)
        count++;

    printf("%ld\n", count);         //5s之后杀掉进程，此时并不执行

    exit(0);
}
