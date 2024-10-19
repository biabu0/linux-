#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<string.h>

#include "mytbf.h"

#define CPS     10              //一秒十个字符
#define BUFSIZE 1024             //1,10,100有什么区别
#define BURST   100               //最多攒够100次权限,令牌筒


/*********

令牌桶示例流控

每次没有数据读取时，就保存一次令牌，等到有数据读取时就可以一次读取令牌数的数据

*************/


int main(int argc, char** argv)
{
    char buf[BUFSIZE];
    int sfd,dfd = 1;
    int ret, len, pos;
    int size;
    mytbf_t *tbf;
    
    if(argc < 2){
        fprintf(stderr, "Usage...\n");
        exit(1);
    }

    tbf = mytbf_init(CPS, BURST);
    if(tbf == NULL)
    {
        fprintf(stderr, "mytbf_init() failed!\n");
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
        size = mytbf_fetchtoken(tbf, BUFSIZE);
        if(size < 0){
            fprintf(stderr, "mytbf_fetchtkoken():%s\n", strerror(-size));
            exit(1);
        }
        while((len = read(sfd, buf,size)) < 0)      //读取到的有效的字节
        {
            if(errno == EINTR)
                continue;
            perror("read()");
            break;
        }
        if(len == 0){
            break;
        }
        
        if(size - len > 0)
            mytbf_returntoken(tbf, size-len);

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

    mytbf_destroy(tbf);
       
    exit(0);
}
