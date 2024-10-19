#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<glob.h>

#define PATHSIZE    1024

/*

实现du部分功能，打印目录下的大小

*/


static int path_noloop(const char *path)
{

    /*判断是否出现循环*/
    char *pos;
    pos = strrchr(path, '/');//返回右侧的‘/’位置
    if(pos == NULL){
        exit(1);
    }

    if(strcmp(pos+1, ".") == 0 || strcmp(pos+1, "..") == 0)
        return 0;

    return 1;
    
}


static int64_t mydu(const char *path)
{
    static struct stat statbuf;//全部在递归点之前使用，可以将变量优化到静态区使用
    glob_t globres;//跨递归点使用
    static char nextpath[PATHSIZE];//
    static int res;
    int64_t sum = 0;

    if(lstat(path, &statbuf) < 0){
        perror("lstat()");
        exit(1);
    }

    if(!S_ISDIR(statbuf.st_mode)){  //不是目录文件
        return statbuf.st_blocks;
    }

    strncpy(nextpath, path, PATHSIZE);
    strncat(nextpath, "/*", PATHSIZE);
    glob(nextpath, 0, NULL, &globres);
    /*if((res = glob(nextpath, 0, NULL, &globres)) != 0)
    {
        printf("glob:error\n");
        exit(1);
    }
*/
    strncpy(nextpath, path, PATHSIZE);
    strncat(nextpath, "/.*", PATHSIZE);//所有隐藏文件
    //隐藏文件以点开头，但隐藏的文件有.和..，分别指向当前目录和上一级目录，从而造成循环
    glob(nextpath, GLOB_APPEND, NULL, &globres);
    /*if((res = glob(nextpath, GLOB_APPEND,NULL, &globres)) != 0)
    {
        printf("glob2:error, %d\n", res);
        exit(1);
    }
*/
    sum += statbuf.st_blocks;//加上当前目录的文件大小
    for(int i = 0; i < globres.gl_pathc; i++){
        if(path_noloop(globres.gl_pathv[i]))//不构成循环时,不加的话，由于不停的递归导致栈破裂，查询栈的大小使用ulimit-a命令
            sum += mydu(globres.gl_pathv[i]);
    }
    
    globfree(&globres);
    return sum;

    
}



int main(int argc , char *argv[])
{

    if(argc < 2){
    
        fprintf(stderr, "Usage....\n");
        exit(1);
    }

    printf("%lld\n", mydu(argv[1])/2);
 
    exit(0);
}
