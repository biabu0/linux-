#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<sys/select.h>
#include<poll.h>
#include<sys/epoll.h>


#define TTY1    "/dev/tty11"
#define TTY2     "/dev/tty12"
#define BUFSIZE     1024

enum
{
    STATE_R = 1,            //枚举使用逗号
    STATE_W,
    STATE_AUTO,             //上方是有条件推动的内容，下方是无条件推动的内容
    STATE_Ex,
    STATE_T
};


struct fsm_st                           //有限状态机的结构体
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];                  //读出来的数据放到缓冲区，用于写
    int len;                            //存读出来的数据大小
    int pos;                            //从buf中的哪个位置开始写（坚持写够）
    char *errstr;                       //出错的来源
};


static void fsm_driver(struct fsm_st *fsm)
{
    int ret;

    switch(fsm->state){
        case STATE_R:
            fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
            if(fsm->len == 0)
                fsm->state = STATE_T;
            else if(fsm->len < 0){
                if(errno == EAGAIN)
                    fsm->state = STATE_R;
                else{
                    fsm->errstr = "read()";
                    fsm->state = STATE_Ex;
                }
            }
            else{
                fsm->pos = 0;
                fsm->state = STATE_W;
            }
            break;
        case STATE_W:
            ret = write(fsm->dfd, fsm->buf+fsm->pos, fsm->len);
            if(ret < 0)
            {
                if(errno == EAGAIN)
                    fsm->state = STATE_W;
                else{
                    fsm->errstr = "write()";
                    fsm->state = STATE_Ex;
                }
            }
            else
            {
                fsm->pos += ret;
                fsm->len -= ret;
                if(fsm->len == 0)
                    fsm->state = STATE_R;
                 else
                    fsm->state = STATE_W;
            }
            break;

        case STATE_Ex:
            perror(fsm->errstr);                    //出错报错
            fsm->state = STATE_T;
            break;

        case STATE_T:
            /*do sth*/
            break;
        default:
            /*do sth*/
            abort();                                //导致进程异常终止
            break;
    }

}


static int max(int a, int b)
{
    if(a>b)
        return a;
    else
        return b;
}

static void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save, epfd;
    struct fsm_st fsm, fsm12, fsm21;    
    struct epoll_event ev;

    fd1_save = fcntl(fd1, F_GETFL);                      //获取属性
    fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);           //确保fd1为非阻塞的（不能集希望于用户直接使用非阻塞）
    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save|O_NONBLOCK);
    
    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;
    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;
    
    epfd = epoll_create(10);                                         //创建的是文件描述符，需要关闭
    if(epfd < 0){
        perror("epoll_create()");
        exit(1);
    }
    
    ev.events = 0;                                                  //fd1, fd2放到epoll的机制中，让epoll管理
    ev.data.fd = fd1;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd1, &ev);                       //在循环外添加示例，这样只需要在循环内修改不同的地方即可
    
    ev.events = 0;
    ev.data.fd = fd2;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd2, &ev);
    while(fsm12.state != STATE_T || fsm21.state != STATE_T){
        //布置监视任务
        
        ev.data.fd = fd1;
        ev.events = 0;

        //epoll_ctl(epfd, );

        if(fsm12.state == STATE_R)                  //将读集和写集初始化
            ev.events |= EPOLLIN;
        if(fsm21.state == STATE_W)
            ev.events |= EPOLLOUT;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd1, &ev);       //将修改后的新的放入

        ev.data.fd = fd2;
        ev.events = 0;
        if(fsm21.state == STATE_R)
            ev.events |= EPOLLIN;
        if(fsm12.state == STATE_W)
            ev.events |= EPOLLOUT;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd2, &ev);


        //监视
        if(fsm12.state < STATE_AUTO || fsm21.state < STATE_AUTO){
            while(epoll_wait(epfd, &ev, 1, -1) < 0){                
                if(errno == EINTR)
                    continue;                               //这里可以使用while，因为poll不需要重新布置现场
                perror("epoll()");
                exit(1);
            }
        }
        //查看监视结果
        if((ev.data.fd == fd1 && ev.events & EPOLLIN) \
            || (ev.data.fd == fd2 && ev.events & EPOLLOUT) \
            || fsm12.state > STATE_AUTO)               //
            fsm_driver(&fsm12);                                    //fsm12时结构体，传地址节省传参
        if((ev.data.fd == fd1 && ev.events & EPOLLOUT) \
            || (ev.data.fd == fd2 && ev.events & EPOLLIN)\
            || fsm21.state > STATE_AUTO)
            fsm_driver(&fsm21);
    }

    fcntl(fd1, F_SETFL, fd1_save);              //将状态恢复
    fcntl(fd2, F_SETFL, fd2_save);
    
    close(epfd);
}

int main()
{
    int fd1, fd2;
    fd1 = open(TTY1, O_RDWR);
    if(fd1 < 0){
        perror("opne()");
        exit(1);
    }
    
    write(fd1, "TTY1\n", 5);

    fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
    if(fd2 < 0){
        perror("open()");
        exit(1);
    }
    
    write(fd2, "TTY2\n", 5);

    relay(fd1, fd2);
    
    close(fd1);
    close(fd2);


    exit(0);
}
