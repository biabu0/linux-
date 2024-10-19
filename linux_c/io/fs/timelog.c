/*
向一个文件中写入时间
1.2024-09-07 15：08：21
2.2024-09-07 15：08：22
...
过十分钟继续写
3.2024-09-07 15：18：22
...

*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define FNAME  "/tmp/out"
#define BUFFSIZE 1024

int main()
{
    FILE *fp;
    char buf[BUFFSIZE];
    int count = 0;
    time_t stamp;
    struct tm *tm;

    fp = fopen(FNAME, "a+");
    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }

    while(fgets(buf, BUFFSIZE, fp) != NULL){
        count++;
    }
        
    while(1)
    {
        time(&stamp);

        tm = localtime(&stamp);

        fprintf(fp, "%-4d%d-%d-%d %d:%d:%d\n", ++count,      //向文件中输入，则当缓冲区充满才会刷新。
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        fflush(fp);     //刷新缓冲区
        sleep(1);       //
        
    }

    fclose(fp);//此时会出现内存泄漏现象，因为要从终端直接结束进程，不会运行到此，需要后续信号处理知识点


    exit(0);
}


