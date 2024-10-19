#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>



int main(int argc, char * argv[])
{
    int fd;
    off_t res;

    if(argc < 2){
    
        fprintf(stderr, "Usage....\n");
        exit(1);
    }

    fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0600);
    
    //res = lseek(fd, 5*1024*1024*1024-1, SEEK_SET);//并不知道数据类型，容易数据益处
    res = lseek(fd, 5LL*1024LL*1024LL*1024LL-1LL, SEEK_SET);//
    if(res == -1){
        perror("lseek()");
        exit(1);
    }

    write(fd, "", 1);

    close(fd);

    exit(0);
}
