#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>

#include"proto.h"

#define IPSTRSIZE   40


int main()
{
    int sd;                     //文件描述符
    struct sockaddr_in  laddr, raddr;              //bind中的第二个参数需要根据不同的协议查找结构体的定义,laddr:本机的地质，raddr， remote，对端的addr
    socklen_t raddr_len;                        //
    char ipstr[IPSTRSIZE];
    struct msg_st rbuf;

    sd = socket(AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/);    //使用AF_INET（IPV4） 协议族中的默认支持报文传递的协议         //取得socket
    if(sd < 0){
        perror("socket()");
        exit(1);
    }

    laddr.sin_family = AF_INET;
    laddr.sin_port =  htons(atoi(RCVPORT));     //与本机约定的addr，IP地质和端口是要经过网络传输的，所以使用hton
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);  //inet_pton将ip地质点分式 192.11....->32位
    
    if(bind(sd, (void *)&laddr, sizeof(laddr)) < 0)            //使用void*转换结构体     //给地质
    {
        perror("bind()");
        exit(1);
    }
    
    /*!!!!!!!!!!!!!!!!!!!!!!!!*/
    raddr_len = sizeof(raddr);      //如果不初始化，第一次会出错，经过通信之后会知道对端的地质是什么样的，后续就不会出错了

    while(1){
        recvfrom(sd, &rbuf, sizeof(rbuf), 0, (void*)&raddr, &raddr_len);
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
        printf("---MESSAGE FROM %s:%d---\n", ipstr , ntohs(raddr.sin_port));       //
        printf("NAME = %s\n", rbuf.name);                   //name是单字节，不涉及大端小端存储情况
        printf("MATH = %d\n", ntohl(rbuf.math));
        printf("CHINESE = %d\n", ntohl(rbuf.chinese));
    }


    close(sd);


    exit(0);
}
