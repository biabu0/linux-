#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define TIMESTRSIZE 1024

int main()
{
    struct tm *tm;
    time_t stamp;
    char timestr[TIMESTRSIZE];


    stamp = time(NULL);
    /* if error*/
    tm = localtime(&stamp);
    strftime(timestr, TIMESTRSIZE, "Now: %Y-%m-%d", tm);
    puts(timestr);

    tm->tm_mday += 100;
    (void)mktime(tm);   //会将不合法的tm类型转换成tm类型，故使用其副作用，调整100以后的年月日
    strftime(timestr, TIMESTRSIZE, "100 day later: %Y-%m-%d", tm);
    puts(timestr);

    exit(1);
}
