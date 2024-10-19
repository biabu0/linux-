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
    sigset_t set, oset, saveset;

    signal(SIGINT, int_handler);        //加入信号，出现SIGINT（中断ctrl+c），跳转到函数，多个信号传输过来，此时也只能由于信号被阻塞而响应一次，因为信号在pedding上表现为位图，重复设为1
    sigemptyset(&set);                  //对集合清空，他是一个auto类型存储的数据类型
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_UNBLOCK, &set, &saveset);       //避免从其他程序传来的信号状态不一致导致最后全部被解除阻塞，先保存原信号状态saveset

    for(int j = 0; j < 1000; j++)
    {
        //sigprocmask(SIG_BLOCK, &set, &oset);     //将集合中的信号阻塞，相当于将mask设置为0
        sigprocmask(SIG_BLOCK, &set, NULL);
        for(int i = 0; i < 10; i++){
            write(1, "*", 1);
            sleep(1);
        }
        write(1, "\n", 1);
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        //sigprocmask(SIG_SETMASK, &oset, NULL);       //解除阻塞
    }

    sigprocmask(SIG_SETMASK, &saveset, NULL);           //恢复信号状态

    exit(0);


}
