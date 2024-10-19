#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<assert.h>


#include "proto.h"

#define BUF_SIZE    10

static char buffer[BUF_SIZE];


typedef enum server_state_e
{
    server_state_start,
    server_state_wait_path,
    server_state_send_data,
    server_state_send_eot,
    server_state_fail,
    server_state_end
}server_state_t;

typedef struct file_reader_st
{
    FILE *hander;
    long offset;                    //文件指针的move
}file_reader_t;


typedef struct server_context_st
{
    server_state_t state;
    int msgqid;
    file_reader_t reader;
    msg_s2c_t send_msg;
    msg_c2s_t recv_msg;
}server_context_t;


int main()
{
    key_t key;
    int len;                //msgrcv返回的数据长度
    int msgid;
    server_context_t ctx;               //定义server文件


    key = ftok(KEYPATH, KEYPROJ);
    if(key < 0){
        perror("ftok()");
        exit(1);
    }

    msgid = msgget(key, IPC_CREAT|0600);
    if(msgid < 0){
        perror("msgget()");
        exit(1);
    }
    
    ctx.msgqid = msgid;
    ctx.state = server_state_start;

    memset(&ctx.send_msg, 0, sizeof(msg_s2c_t));
    memset(&ctx.recv_msg, 0, sizeof(msg_c2s_t));
    memset(&ctx.reader, 0, sizeof(file_reader_t));

    while(ctx.state != server_state_end){
        
        switch(ctx.state){
            case server_state_start:
                ctx.state = server_state_wait_path;
                break;
            case server_state_wait_path:
                len = msgrcv(ctx.msgqid, &ctx.recv_msg, sizeof(ctx.recv_msg), 0, 0);
                if(len < 0){
                    ctx.state = server_state_fail;
                }
                else{
                    assert(ctx.recv_msg.mtype == MSG_PATH);
                    assert(ctx.reader.hander == NULL);

                    FILE *f = fopen(ctx.recv_msg.msg_path.path, "rb");          //open file
                    if(f == NULL){
                        ctx.state = server_state_fail;
                    }
                    else{
                        ctx.reader.hander = f;
                        ctx.reader.offset = 0;
                        ctx.state = server_state_send_data;

                    }
                }
                break;
            case server_state_send_data:
                assert(ctx.reader.hander != NULL);
                
                if(fseek(ctx.reader.hander, ctx.reader.offset, SEEK_SET) == 0){
                    memset(buffer, 0, BUF_SIZE);

                    size_t bytes_read = fread(buffer, sizeof(char), BUF_SIZE, ctx.reader.hander);

                    if(bytes_read > 0){
                        memset(&ctx.send_msg, 0, sizeof(msg_s2c_t));
                        ctx.send_msg.mtype = MSG_DATA;
                        memcpy(&ctx.send_msg.msg_data.data, buffer, BUF_SIZE);             //将读取的数据准备发送
                        if(msgsnd(ctx.msgqid, &ctx.send_msg, sizeof(msg_s2c_t), 0) < 0){
                            ctx.state = server_state_fail;
                        }
                        else{
                            ctx.state = server_state_send_data;
                            ctx.reader.offset += bytes_read;
                        }
                    }
                    else{                                                               //假设读不会出错        
                        ctx.state = server_state_send_eot;
                    }
                }
                else{
                    ctx.state = server_state_fail;
                }
                break;
            case server_state_send_eot:
                memset(&ctx.send_msg, 0, sizeof(msg_s2c_t));
                ctx.send_msg.mtype = MSG_EOT;

                if(msgsnd(ctx.msgqid, &ctx.send_msg, sizeof(msg_s2c_t), 0) < 0){
                    ctx.state = server_state_fail;
                }
                else{
                    if(ctx.reader.hander != NULL){              //传送完成后关闭文件
                        fclose(ctx.reader.hander);
                    }
                    ctx.state = server_state_end;
                }
                break;
            case server_state_fail:
                fprintf(stdout, "send date to client failed for unknown reason.");
                if((ctx.reader.hander != NULL)){
                    fclose(ctx.reader.hander);
                }
                ctx.state = server_state_end;
                
                break;
        
            default:
                ctx.state = server_state_end;
                break;
        }
        
    }


    exit(0);
    
}
