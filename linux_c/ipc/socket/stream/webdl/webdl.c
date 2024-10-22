#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>

#include"proto.h"
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in raddr;
    socklen_t len;
    FILE *fp;
    long long stamp;
    char rbuf[BUFSIZE];
    int len1;

    if(argc < 2){
        fprintf(stderr, "Usage..........\n");
        exit(1);
    }

    sd = socket(AF_INET,SOCK_STREAM,0);
    if(sd < 0){
        perror("socket()");
        exit(1);
    }

//    bind();
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(80);             //网络端口
    inet_pton(AF_INET,argv[1], &raddr.sin_addr);
    len = sizeof(raddr);
    if(connect(sd, (void *)&raddr, len) < 0){
        perror("connect()");
        exit(1);
    }
    
    fp = fdopen(sd, "r+");
    if(fp == NULL){
        perror("fdopen()");
        exit(1);
    }

    fprintf(fp, "GET /test.jpeg\r\n\r\n");
    fflush(fp);

    while(1){
        len1 = fread(rbuf, 1, BUFSIZE, fp);
        if(len <= 0)
            break;
        fwrite(rbuf, 1, len1, stdout);
    }

    fclose(fp);






//  recv();
//  close();

    exit(1);
}
