#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

/*
* date +%s
*/

int main()
{

    puts("Begin!");     //puts自动换行
    fflush(NULL);       //exec之前需要刷新缓冲区，不然全缓冲才刷新，此时调用exec产生了新的进程映像替换了旧的进程映像,导致execl之前的内容出现问题。
    execl("/bin/date", "date", "+%s", NULL);{
        perror("execl()");
        exit(1);
    }
    puts("End!\n");//新的进程映像替换了现在的，此处不会打印
    exit(0);
}


