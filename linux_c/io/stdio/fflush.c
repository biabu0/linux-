#include<stdio.h>
#include<stdlib.h>

/*
 




*/
int main()
{
    printf("before");//无换行，不能刷新

    fflush(stdout);

    while(1);

    printf("after");
    fflush(NULL);//刷新所有打开的流
  

    exit(0);
}
