#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

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

    sfd = open(argv[1],O_RDONLY);
    if(sfd < 0){
        perror("open()");
        exit(1);
    }
    dfd = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if(dfd < 0){
        close(sfd);//已经打开了sfd
        perror("open()");
        exit(1);
    }
    
    while(1)
    {
        len = read(sfd, buf, BUFSIZE);//读取到的有效的字节
        if(len < 0){
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
