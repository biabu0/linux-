#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>

static void int_handler(int s)
{
    write(1, "!", 1);
}


int main()
{
    
    signal(SIGINT, int_handler);        //加入信号，出现SIGINT（中断ctrl+c），跳转到函数：
    
    for(int i = 0; i < 10; i++){
        write(1, "*", 1);
        sleep(1);
    }

    exit(0);


}
