#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>

static off_t flen(const char *fname)//不能直接选择int类型的返回，根据课本，应为off_t类型
{
    struct stat statres;

    if(stat(fname, &statres) < 0){
        perror("stat()");
        exit(1);
    }

    return statres.st_size;

    
}


int main(int argc, char *argv[])
{
    
    if(argc < 2)
    {
        fprintf(stderr, "Usage.....\n");
        exit(1);
    }

    printf("%lld\n",(long long)flen(argv[1]));//强转


    exit(0);
}
