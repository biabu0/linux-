#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<syslog.h>
#include<errno.h>
#include<string.h>
#include<signal.h>

#define FNAME "/tmp/out"

/**使用/var/log/syslog查看日志文件**/

FILE *fp;           //  定义成全局变量

static int daemonize(void)
{
    pid_t pid;
    int fd;

    pid = fork();
    if(pid < 0){
        return -1;
    }
    if(pid > 0){
        exit(0);
    }
    
    fd = open("/dev/null", O_RDWR);  //打开空设备
    if(fd < 0){
        return -1;
    }
    dup2(fd, 0);        //重定向，将0，1，2重定向到指定的fd
    dup2(fd, 1);
    dup2(fd, 2);
    if(fd > 2)
        close(fd);

    setsid();

    chdir("/");         //将守护进程工作路径改

    //umask(0);           //程序不会产生文件

    return 0;

}

static void daemon_exit(int s)
{   
    /***
    *不同的信号处理函数则可以
    if(s == SIGINT){
    
    }
    if(s == SIGQUIT){
    
    }
    *
    **/
    fclose(fp);
    closelog();
    exit(0);
}


int main()
{

    struct sigaction sa;
    
    sa.sa_handler = daemon_exit;        //信号处理函数
    sigemptyset(&sa.sa_mask);           //将该信号集设为空
    sigaddset(&sa.sa_mask, SIGQUIT);    //向该信号集合增加需要额外阻塞的信号
    sigaddset(&sa.sa_mask, SIGTERM);    
    sigaddset(&sa.sa_mask, SIGINT);
    sa.sa_flags = 0;                    //没有特殊要求

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT,&sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
//    signal(SIGINT, daemon_exit); // 在一个信号响应的时候，其他信号暂时阻塞不响应，此时需要使用sigprocmask函数
//    signal(SIGQUIT, daemon_exit);//这是相对于sigaction较为区别的
//    signal(SIGTERM. daemon_exit);

    openlog("mudaemon", LOG_PID, LOG_DAEMON);

    if(daemonize()){
        syslog(LOG_ERR, "daemonize() failed!");         //不需要加/n
        exit(1);
        
    }
    else{
        syslog(LOG_INFO, "daemonize() successed!");
    }

    /*******守护进程的作用：向文件中一秒钟写一个数据*********/

    fp = fopen(FNAME, "w");

    if(fp == NULL){
        syslog(LOG_ERR, "fopen():%s", strerror(errno));
        exit(1);
    }

    syslog(LOG_INFO, "%s was opened.", FNAME);

    for(int i = 0; ; i++){
        fprintf(fp,"%d\n", i);//对于标准输出使用\n是行缓冲，但此时是向文件中输入，全缓冲模式，需要刷新
        
        syslog(LOG_DEBUG, "%d is printed.", i);
        fflush(fp);
        sleep(1);
    }

//    fclose(fp);             //无法执行到此，无法释放资源
//    closelog();             //同上,使用信号重构

    exit(0);
}
