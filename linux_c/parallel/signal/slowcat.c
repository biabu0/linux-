#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<sys/time.h>

#define CPS     10              //一秒十个字符
#define BUFSIZE CPS             //1,10,100有什么区别

/********************漏筒示例，秒打印10个字符，无论数据多大；当没有数据的时候等待************************/

static volatile int loop = 0;           //volatile 动态变化的

static void alrm_handler(int s)
{
    //alarm(1);                   //alarm链
    loop = 1;
}


int main(int argc, char** argv)
{
    char buf[BUFSIZE];
    int sfd,dfd = 1;
    int ret, len, pos;
    struct itimerval itv;
    
    if(argc < 2){
        fprintf(stderr, "Usage...\n");
        exit(1);
    }

    signal(SIGALRM, alrm_handler);

    //alarm(1);
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &itv, NULL) < 0)
    {
        perror("setitimer()");
        exit(1);
    }
    

    do{
    sfd = open(argv[1],O_RDONLY);               //阻塞的系统调用
    if(sfd < 0){
        if(errno != EINTR){                     //EINTR:信号打断
            perror("open()");
            exit(1);
        }

    }
    }while(sfd < 0);                        //假的错误则继续重新打开
    

    while(1)
    {
        while(!loop)
            pause();

        loop = 0;

        while((len = read(sfd, buf, BUFSIZE)) < 0)      //读取到的有效的字节
        {
            if(errno == EINTR)
                continue;
            perror("read()");
            break;
        }
        if(len == 0){
            break;
        }
        pos = 0;//假设len&ret不想等，为了避免遗漏没有写入的字符
        while(len > 0)//坚持写够len个字节
        {
            ret = write(dfd, buf, len);//例如要写10个字节，但之成功写入3个字节，如此，下一个循环就会从buf+3的位置继续读入
            if(ret < 0){
                if(errno == EINTR)
                    continue;
                perror("write()");
                exit(1);
            }
            pos += ret;
            len -= ret;
        }
        //sleep(1);                       //不能发布在源码中，在一些平台上有问题

    }


    close(dfd);
       
    exit(0);
}
