#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<syslog.h>
#include<errno.h>
#include<string.h>

#define FNAME "/tmp/out"


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


int main()
{
    FILE *fp;
    
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


    fclose(fp);
    exit(0);
}
