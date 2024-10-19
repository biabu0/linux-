/*指定一个文件，输出a有多少个字符*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>

int main(int argc, char *argv[])
{
    int fd;
    struct stat statres;
    char *str;
    int i;
    int64_t count = 0;

    if(argc < 2){
        fprintf(stderr, "Usage....\n");
        exit(1);
    }
    
    fd = open(argv[1], O_RDONLY);
    if(fd < 0){
        perror("open()");
        exit(1);
    }

    if(fstat(fd, &statres) < 0){                            //stat()获取文件属性，从中取出文件宗字节数
        perror("fstat()");
        exit(1);
    }




    str = mmap(NULL, statres.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(str == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }

    close(fd);                                              //已经将文件中的数据映射到了空间中，不再需要，可以关闭了

    for(i = 0; i < statres.st_size; i++){
        if(str[i] == 'a')
            count++;
    }

    printf("%ld\n", count);




    munmap(str, statres.st_size);

    exit(0);
}
