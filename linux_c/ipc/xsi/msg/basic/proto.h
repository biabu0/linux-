// 制作协议（本机状态下两个进程通信）
#ifndef PROTO_H__
#define PROTO_H__

#define KEYPATH     "/etc/services"
#define KEYPROJ     'g'             //int类型，没有单位的数没有意义，使用字符可以转二进制的ASCII码


#define NAMESIZE    1024

//  约定传输数据的格式
struct msg_st
{
    long mtype;             //msgget()函数中的存储的数据结构 void *msgp
    char name[NAMESIZE];        //char mtext[1],数据格式不一定就要按照给出的
    int math;
    int chinese;
};


#endif
