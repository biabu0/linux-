#ifndef PROTO_H__
#define PROTO_H__

#define RCVPORT "1989"                    //接受方ip地质放到argv指定，这里指定端口，1024以内端口yuliu, ”1989“，没有单位的数字没有意义，""，后面使用的时候使用atoi()


#define NAMEMAX   512-8-8             //512:UDP推荐长度， 8：UDP报头，8：其他定长数据长度

struct msg_st
{   
    uint32_t math;
    uint32_t chinese;
    uint8_t name[1];                    //结构体变长，name 要一个占位符

}__attribute__((packed));                 //告诉编译器buduiqi 







#endif
