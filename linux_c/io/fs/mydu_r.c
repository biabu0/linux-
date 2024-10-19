#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>

#define PATHSIZE    1024


static int  path_noloop(const char *path)
{
    char *pos;
    pos = strrchr(path, '/');
    if(pos == NULL)
        exit(1);
    
    if(strcmp(pos+1, ".") == 0 || strcmp(pos+1, "..") == 0)
        return 0;
    return 1;
}


static int64_t mydu(const char *path)
{
    int64_t sum = 0;//long long 类型
    static struct stat statbuf;
    DIR *pd;
    struct dirent *read;
    char nextpath[PATHSIZE];

    if(lstat(path, &statbuf) < 0){  //报错信息忘写
        perror("lstat()");
        exit(1);
    }//使用lstat，可以获取符号文件的属性
    if(!S_ISDIR(statbuf.st_mode))
        return statbuf.st_blocks;
    
    pd = opendir(path);
    if(pd == NULL){
        perror("opendir()");
        exit(1);
    }
    
    sum += statbuf.st_blocks;
    while((read = readdir(pd)) != NULL){
        if(strcmp(read->d_name, ".") && strcmp(read->d_name, "..")){
            strncpy(nextpath, path, PATHSIZE);
            strncat(nextpath, "/", PATHSIZE);
            strncat(nextpath, read->d_name, PATHSIZE);
            //if(path_noloop(nextpath))
            sum += mydu(nextpath);
            }
    }
    closedir(pd);
    return sum;

}

int main(int argc, char *argv[])
{

    if(argc < 2){
        fprintf(stderr, "Usage...\n");
        exit(1);
    }

    printf("%lld\n",mydu(argv[1])/2);


    exit(0);
}

