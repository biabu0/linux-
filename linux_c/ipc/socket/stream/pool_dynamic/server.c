#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
#include <sys/mman.h>


#include"proto.h"
#define BUFSIZE 1024

/*      资源上下限的定义        */
#define MINSPARESERVER  5       //最小支持的空闲个数
#define MAXSPARESERVER  10      //最大支持的空闲个数
#define MAXCLIENTS          20          //客户端数量//最多有20个子进程并发
#define SIG_NOTIFY      SIGUSR2 //预留给用户定义行为的信号
#define IPSTRSIZE   40
#define LINEBUFSIZE 80
enum
{
    STATE_IDLE = 0,
    STATE_BUSY
};

//20个，可以使用链式存储或者数组
struct server_st
{
    pid_t pid;          //pid没有用到则设置为-1
    int state;          //pid 有效数值时使用
    //int resue 计数 某个子进程重复使用次数超过上限则杀掉该子进程，重新创建一个子进程；程序中可能有Bug，时间越长，bug出现的概率更高；每次接受一个c端口的连接时，则一个变量自增，当达到上限时父进程kill掉，保证该子进程要没有c端连接才能kill；
    
};

//使用堆存储
static struct server_st *serverpool;
static int idle_count = 0, busy_count = 0;
static int sd;


static void server_job(int pos)
{
    int ppid;
    struct sockaddr_in raddr;
    socklen_t raddr_len;
    int client_sd;
    int len;
    size_t send_len;
    long long int stamp;
    char ipstr[IPSTRSIZE];
    char linebuf[LINEBUFSIZE];

    ppid = getppid();
    
    while(1){
        serverpool[pos].state = STATE_IDLE;
        kill(ppid, SIG_NOTIFY);         //通知父进程，确认当前状态
        
        client_sd = accept(sd, (void *)&raddr, &raddr_len);
        if(client_sd < 0){
            if(errno != EINTR || errno != EAGAIN){
                perror("accept()");
                exit(1);
            }
        }
        serverpool[pos].state = STATE_BUSY;
        kill(ppid, SIG_NOTIFY);
        inet_ntop(AF_INET, &raddr.sin_addr,ipstr, IPSTRSIZE);
//        printf("[%d]cilent:%s:%d\n", getpid(),ipstr, ntohs(raddr.sin_port));
        stamp = time(NULL);
        len = snprintf(linebuf, LINEBUFSIZE, FMT_STAMP, stamp);
        send_len = send(client_sd, linebuf, len, 0);
        if(send_len < 0){
            perror("send()");
            exit(1);
        }
        sleep(5);
        close(client_sd);

    }    
}

//只需要发一个信号，驱动程序
static void usr2_handler(int s)
{
    return ;
}

static int add_1_server(void)
{
    int slot;
    int pid;

    if(idle_count + busy_count >= MAXCLIENTS)
        return -1;
    for(slot = 0; slot < MAXCLIENTS; slot++)
        if(serverpool[slot].pid == -1)              //类似getfreepos,找一个没有使用的
            break;

    serverpool[slot].state = STATE_IDLE;
    pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }
    if(pid == 0){
        server_job(slot);               //取时间戳，发送给客户端
                                    //此处是否应该给状态busy？？？？？？？？？？？？？？？
        exit(0);
    }

    else{
        serverpool[slot].pid = pid;
        idle_count ++;
    }

    return 0;


}

static int del_1_server(void)
{
    if(idle_count == 0)             //空闲状态的进程为0，则无法继续杀死
        return -1;
    for(int i = 0; i < MAXCLIENTS; i++){
        if(serverpool[i].pid != -1 && serverpool[i].state == STATE_IDLE){
            kill(serverpool[i].pid, SIGTERM);               //将SIGTERM（终止信号）发送给pid进程
            serverpool[i].pid = -1;
            idle_count --;
            break;
        }
    }
    return 0;
}


static int scan_pool(void)
{
    int idle = 0,busy = 0;
    int i;
    for(i = 0; i < MAXCLIENTS; i++){
        if(serverpool[i].pid == -1)                 //当前位置没有启用
            continue;
        if(kill(serverpool[i].pid, 0))  //检测是否子进程消亡但空间没有空出来
        {
            serverpool[i].pid = -1;
            continue;
        }
        if(serverpool[i].state == STATE_IDLE)
            idle++;
        else if(serverpool[i].state == STATE_BUSY)
            busy++;
        else    
        {
            fprintf(stderr, "Unkonwn state.\n");
        //    _exit(1);
            abort();//杀掉进程
        }
    }

    idle_count = idle;          //idle_count是全局变量；如此产生冲突的几率较小
    busy_count = busy;

}

int main()
{
    struct sigaction sa, osa;
    int val;
    struct sockaddr_in laddr;
    sigset_t set,oset;    
    
    //下述代码使得子进程结束后自行消亡，不需要父进程等待收拾
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;             //阻止子进程变成僵尸状态
                            //SIGCHLD:一个进程终止或者停止时，该信号被发送给父进程
    sigaction(SIGCHLD, &sa, &osa);
    

    //
    sa.sa_handler = usr2_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIG_NOTIFY, &sa, &osa);
    

    sigemptyset(&set);
    sigaddset(&set, SIG_NOTIFY);
    sigprocmask(SIG_BLOCK, &set, &oset);                //将该信号加入阻塞信号集合中


    //可以使用malloc，这里使用存储映射IO
    serverpool = mmap(NULL, sizeof(struct server_st) * MAXCLIENTS, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(serverpool == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }

    for(int i = 0; i < MAXCLIENTS; i++){
        serverpool[i].pid = -1;
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }
    

    //防止程序意外终止，下次使用bind连接不上地址
    if(setsockopt(sd, SOL_SOCKET,SO_REUSEADDR, &val, sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
    if(bind(sd, (void*)&laddr, sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    if(listen(sd, 100) < 0){
        perror("listen()");
        exit(1);
    }

    
    for(int i = 0; i < MINSPARESERVER; i++){
        add_1_server();
        
    }
    
    //信号驱动程序：当进程池内有进程状态发生改变的时候，发送一个信号给父进程
    while(1){
        
        sigsuspend(&oset);          //当oset阻塞集合之外的信号到来的时候，唤醒当前的操作；接受SIG_NOTIFY信号，并处理该信号

        scan_pool();                    //遍历池内的状态

        //contrl the pool

        if(idle_count > MAXSPARESERVER)
            for(int i = 0; i < (idle_count - MAXSPARESERVER); i++)
                del_1_server();                         //进程池内空闲的进程太多，杀掉空闲的
        else if(idle_count < MINSPARESERVER)
            for(int i = 0; i < (MAXSPARESERVER - idle_count); i++)
                add_1_server();
           
            //输出当前池的状态
        for(int i = 0; i < MAXCLIENTS; i++){
            
            if(serverpool[i].pid == -1)
                putchar(' ');                       //未启用：空格
            else if(serverpool[i].state == STATE_IDLE)
                putchar('.');                       //空闲：.
            else
                putchar('x');                       //忙碌：x
        }
        putchar('\n');


    }
    

    sigprocmask(SIG_SETMASK, &oset, NULL);              //恢复



    
//    munmap();



    exit(1);
}
