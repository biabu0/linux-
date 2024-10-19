#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>

#define PAH "/etc/"

/*******************************
        打印目录下的所有文件

**********************************/
int main()
{
    
    DIR *pd;
    struct dirent *read;

    pd = opendir(PAH);

    while((read = readdir(pd)) != NULL)
        puts(read->d_name);
    


    closedir(pd);


    exit(1);
}
