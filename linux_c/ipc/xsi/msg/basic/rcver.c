#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<signal.h>

#include"proto.h"

static int msgid;

static void msg_rmid(int s)
{
    msgctl(msgid, IPC_RMID, NULL);
    exit(1);
}


int main()
{
    key_t key;

    struct msg_st msgbuf;

    key = ftok(KEYPATH, KEYPROJ);
    if(key < 0){
        perror("ftok()");
        exit(1);
    }
    msgid = msgget(key, IPC_CREAT|0600);     //CREAT 先运行       //位图，要给一个权限
    if(msgid < 0){
        perror("msgget()");
        exit(1);
    }
    while(1){
        if(msgrcv(msgid, &msgbuf, sizeof(msgbuf)-sizeof(long), 0, 0) < 0){  //first:0从第几个数据开始
            perror("msgrcv()");
            exit(1);
        }
        printf("NAME = %s\n", msgbuf.name);
        printf("MATH = %d\n", msgbuf.math);
        printf("CHINESE = %d\n", msgbuf.chinese);
        
        signal(SIGINT, msg_rmid);               //使用信号销毁
        signal(SIGQUIT, msg_rmid);
        signal(SIGTERM, msg_rmid);
    }
    signal(SIGINT, msg_rmid);
    signal(SIGQUIT, msg_rmid);
    signal(SIGTERM, msg_rmid);

}
