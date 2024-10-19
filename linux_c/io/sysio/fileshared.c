#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


#define BUFSIZE  128

int main(int argc, char *argv[])
{   
    int fd1, fd2;
    int trun_num = 0;//文件总字节数
    int row = 0, rpos = 0, wpos= 0, ret = 0;

    char buf[BUFSIZE];
    if(argc < 2){
        fprintf(stderr, "Usage.......");
        exit(1);
    }

    fd1 = open(argv[1], O_RDONLY);
    if(fd1 < 0){
        perror("open()");
        exit(1);
    }

    fd2 = open(argv[1], O_RDWR);
    if(fd2 < 0){
        close(fd1);//避免内存泄漏
        perror("open()");
        exit(1);
    }

    row = 0;
    printf("before 1 while\n");
    while(1){

        if(row == 10){
            lseek(fd1, rpos, SEEK_SET);
            break;
        }
    
        ret = read(fd1, buf, 1);
        if(ret < 0){
            perror("read()");
            break;
        }

        rpos++;
        if(buf[0] == '\n'){//一个换行符号就是一行
            row++;
        }
        
    }

    printf("after 1 while\n");
    row = 0;
    while(1){

        if(row == 9){
        
            lseek(fd2, wpos, SEEK_SET);
            break;
        }

        if(read(fd2, buf, 1) < 0) break;

        if(buf[0] == '\n'){
            row++;
        }

        wpos++;
    }
    printf("after 2 while\n");
    while(1){
       // if(read(fd1, buf, 1) <= 0) break;//<=0，读取一个字节表示读取完毕
        if(read(fd1, buf, 1) == 0) break;
        if(read(fd1, buf, 1) < 0){
            perror("read()");
            break;
        
        write(fd2, buf, 1);
    }

    printf("after 3 while\n");
    trun_num = lseek(fd2, 0, SEEK_END);

    ftruncate(fd2, trun_num);
    
    
    close(fd1);
    close(fd2);

    exit(0);    
}
