#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<unistd.h>

#define TIMESTRSIZE 1024
#define FMTSTRSIZE 1024
/*
    命令行参数解析
    -y -m -d -H -M -S

*/



int main(int argc, char *argv[])
{
    struct tm *tm;
    time_t stamp;
    char timestr[TIMESTRSIZE];
    int c;
    char fmtstr[FMTSTRSIZE];
    FILE *fp = stdout;


    fmtstr[0] = '\0';
    stamp = time(NULL);
    /* if error*/
    tm = localtime(&stamp);
    
    while(1){
    
        c = getopt(argc, argv, "-H:MSy:md");        //带:是有自己的参数选项，-识别非选项的传参
        if(c < 0)
            break;

        switch(c){
            case 1:
                if(fd == stdout)    //指向第一个读取到的文件写入数据
                {
                    fp = fopen(argv[optind-1], "w"); //optind 指向当前c的下一个下索引
                    if(fp == NULL){
                        perror("fopen()");
                        fp = stdout;
                    }
                }
                break;

            case 'H':
                if(strcmp(optarg, "12") == 0)
                    strncat(fmtstr, "%I(%P) ", FMTSTRSIZE);
                else if(strcmp(optarg, "24") == 0)
                    strncat(fmtstr, "%H ", FMTSTRSIZE);
                else
                    fprintf(stderr, "Invalid arg H\n");
               
                break;
            case 'M':
                strncat(fmtstr, "%M ", FMTSTRSIZE);
                break;
        
        
        }
    
    }
    
    strncat(fmtstr, "\n", FMTSTRSIZE);
    strftime(timestr, TIMESTRSIZE, fmtstr, tm);
    fputs(timestr, fp);

    if(fp != stdout)
        fclose(fd);
/*
    tm->tm_mday += 100;
    (void)mktime(tm);   //会将不合法的tm类型转换成tm类型，故使用其副作用，调整100以后的年月日
    strftime(timestr, TIMESTRSIZE, "100 day later: %Y-%m-%d", tm);
    puts(timestr);
*/
    exit(1);
}
