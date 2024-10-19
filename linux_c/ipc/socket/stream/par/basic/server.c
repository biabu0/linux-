#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>
#include<sys/wait.h>

#include"proto.h"
#define IPSTRSIZE   40

#define BUFSIZE     1024

static void server_job(int sd)
{
    char buf[BUFSIZE];
    int len;
    len = sprintf(buf, FMT_STAMP, (long long)time(NULL));
    if(send(sd, buf, len, 0) < 0){
        perror("send()");
        exit(1);
    }

}

int main()
{

    int sd, newsd;
    struct sockaddr_in laddr, raddr;
    socklen_t raddr_len;
    char ipstr[IPSTRSIZE];
    pid_t pid;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0){
        perror("socket()");
        exit(1);
    }
    
    /*使用SO——REUSEADDR， bind时候，发现端口没有释放，马上释放并连接*/
    int val = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(SERVERPORT));               //该地址需要上传网络
    /*0.0.0.0:“我希望这个服务器套接字能够监听这台机器上的所有IPv4地址。” 这意味着，无论客户端尝试连接到服务器的哪个IPv4地址（前提是该地址已分配给机器），连接都会被接受。*/
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr); //不需要使用hton，inet_pton已经处理了

    if(bind(sd, (void *)&laddr, sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }


    if(listen(sd, 200) < 0){
        perror("listen()");
        exit(1);
    }           
    
    raddr_len = sizeof(raddr);
    while(1)
    {
        newsd = accept(sd, (void *)&raddr, &raddr_len);     //将地质回填到raddr
        if(newsd < 0)
        {
            if(errno == EAGAIN ||errno == EINTR){
                continue;
            }
            else {
                perror("accept()");
                exit(1);
            }
        }
        pid = fork();
        if(pid < 0){
            perror("fork()");
            exit(1);
        }

        //子进程和父进程都会有sd 和newsd
        if(pid == 0){
            close(sd);          //将不需要使用的关闭
            inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
            printf("Client:%s:%d\n", ipstr, ntohs(raddr.sin_port));

            server_job(newsd);
            close(newsd);           //关闭newsd结束和cilent的连接，建立新的连接
            exit(0);
        }
        close(newsd);

    }
    
    wait(NULL);
    close(sd);

    exit(0);
 }
