#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>

#include"proto.h"

int main(int argc, char *argv[])
{
    int sd;
    int size;
    struct msg_st *sbufp;               //不是占512-8-8，所以要使用指针动态分配内存
    struct sockaddr_in  raddr;

    /*./snder IP NAME*/
    if(argc < 3){
        fprintf(stderr, "Usage.....\n");
        exit(1);
    }
    
    size = sizeof(struct msg_st) + strlen(argv[2]);
    sbufp = malloc(size);        //内存空间大小变化
    if(sbufp == NULL){
        perror("malloc()");
        exit(1);
    }

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0){
        perror("socket()");
        exit(1);
    }
    
    if(strlen(argv[2]) > NAMEMAX){
        fprintf(stderr, "NAME is too long!\n");         //名字太长会导致有些数据无法传输，数据包被冲破
        exit(1);
    }

    strcpy(sbufp->name , argv[2]);
    sbufp->math = htonl(rand()%100);
    sbufp->chinese = htonl(rand()%100);
    
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(RCVPORT));                  //对端端口
    inet_pton(AF_INET, argv[1], &raddr.sin_addr);

    if(sendto(sd, sbufp, size, 0, (void *)&raddr, sizeof(raddr)) < 0){
        perror("sendto()");
        exit(1);
    }

    puts("OK!");

    close(sd);
    free(sbufp);            //销毁

    exit(0);
}
