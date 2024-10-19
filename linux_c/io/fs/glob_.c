#include<stdio.h>
#include<stdlib.h>
#include<glob.h>

#define PAT "/etc/a*.conf" // 解析目录

int main()
{
    
    glob_t globres;
    int res, i;

    res = glob(PAT,0, NULL, &globres);
    if(res){
    
        printf("Error code = %d\n", res);
        exit(1);
    }

    for(i = 0; i < globres.gl_pathc; i++)
    {
        puts(globres.gl_pathv[i]);
    }
    
    globfree(&globres);//销毁

    exit(0);
}
