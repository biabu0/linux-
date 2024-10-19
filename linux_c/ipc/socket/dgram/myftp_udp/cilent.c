#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<assert.h>
#include"proto.h"
#define FILEPATH "/home/wyl/linux_qianru/linux_c/ipc/xsi/msg/myftp_msg/data.txt"

typedef enum cilent_state_e
{
    cilent_state_start,
    cilent_state_wait_data,
    cilent_state_send_path,
    cilent_state_recv_data,
    cilent_state_recv_eot,
    cilent_state_fail,
    cilent_state_end,
}cilent_state_t;


typedef struct cilent_context_st
{
    int msgqid;
    cilent_state_t state;
    msg_s2c_t recv_msg;
    msg_c2s_t send_msg;
}cilent_context_t;


int main()
{
    int key;
    int msgid;
    cilent_context_t ctx;

    key = ftok(KEYPATH, KEYPROJ);
    if(key < 0){
        perror("ftok()");
        exit(1);
    }

    msgid = msgget(key, 0);
    if(msgid < 0){
        perror("msgget()");
        exit(1);
    }
    ctx.msgqid = msgid;
    ctx.state = cilent_state_start;
    memset(&ctx.send_msg, 0, sizeof(msg_c2s_t));
    memset(&ctx.recv_msg, 0, sizeof(msg_s2c_t));
    

    while(ctx.state != cilent_state_end){
        switch(ctx.state){
            case cilent_state_start:
                ctx.send_msg.mtype = MSG_PATH;
                strncpy(ctx.send_msg.msg_path.path, FILEPATH, PATHMAX);
                ctx.state = cilent_state_send_path;
                break;

            case cilent_state_send_path:
                if(msgsnd(ctx.msgqid, &ctx.send_msg, sizeof(msg_c2s_t), 0) < 0){
                    ctx.state = cilent_state_fail;
                }
                else{
                    ctx.state = cilent_state_wait_data;
                }
                break;

            case cilent_state_wait_data:                    //接受数据，判断数据
                memset(&ctx.recv_msg, 0, sizeof(msg_s2c_t));

                size_t len = msgrcv(ctx.msgqid, &ctx.recv_msg, sizeof(msg_s2c_t), 0, 0);
                if(len < 0){
                    ctx.state = cilent_state_fail;
                }
                else{
                    assert(ctx.recv_msg.mtype == MSG_DATA | ctx.recv_msg.mtype == MSG_EOT);
                    if(ctx.recv_msg.mtype == MSG_DATA){
                        ctx.state = cilent_state_recv_data;
                    }
                    else{
                        ctx.state = cilent_state_recv_eot;
                    }
                }
                break;

            case cilent_state_recv_data:
                assert(ctx.recv_msg.mtype == MSG_DATA);
                fprintf(stdout, "recv data:\n%s\n", ctx.recv_msg.msg_data.data);            //
                ctx.state = cilent_state_wait_data;             //继续等待数据传输
                break;

            case cilent_state_recv_eot:
                assert(ctx.recv_msg.mtype == MSG_EOT);
                fprintf(stdout, "recv data end.\n");
                ctx.state = cilent_state_end;
                break;

            case cilent_state_fail:
                fprintf(stdout, "receive data from service failed for unkonwn reason");
                ctx.state = cilent_state_end;
                break;

            deault:
                ctx.state = cilent_state_end;
                break;
        }
    }   
    exit(0);
}
