#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>

#define BUFSIZE 1024//1,10,100有什么区别

int main(int argc, char** argv)
{
    char buf[BUFSIZE];
    int sfd,dfd;
    int ret, len, pos;
    
    if(argc < 3){
        fprintf(stderr, "Usage...\n");
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

    do
    {
    dfd = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if(dfd < 0){
        if(errno != EINTR){
            close(sfd);//已经打开了sfd
            perror("open()");
            exit(1);
        }
    }
    }while(dfd < 0);
    
    while(1)
    {
        len = read(sfd, buf, BUFSIZE);//读取到的有效的字节
        if(len < 0){
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
    }


    close(dfd);
    close(sfd);



    exit(0);
}
