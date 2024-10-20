#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<net/if.h>


#include"proto.h"

int main()
{
    int sd;
    struct msg_st sbuf;
    struct sockaddr_in  raddr;
    int val = 1;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0){
        perror("socket()");
        exit(1);
    }
    
    struct ip_mreqn mreq;
    inet_pton(AF_INET, MTGROUP, &mreq.imr_multiaddr);
    inet_pton(AF_INET,"0.0.0.0", &mreq.imr_address);
    mreq.imr_ifindex = if_nametoindex("ens33");         //使用ip ad sh查看网卡信息，使用函数if_nametoindex提取设备索引号
    /*打开组播开关*/
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) < 0){
        perror("setsockopt()");
        exit(1);
    }
    
    

    strcpy(sbuf.name , "Alaon");
    sbuf.math = htonl(rand()%100);
    sbuf.chinese = htonl(rand()%100);
    
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(RCVPORT));                  //对端端口
    inet_pton(AF_INET, MTGROUP, &raddr.sin_addr);       //向多播地质发送

    if(sendto(sd, &sbuf, sizeof(sbuf), 0, (void *)&raddr, sizeof(raddr)) < 0){
        perror("sendto()");
        exit(1);
    }

    puts("OK!");

    close(sd);


    exit(0);
}
