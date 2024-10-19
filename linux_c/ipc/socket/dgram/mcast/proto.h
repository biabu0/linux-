#ifndef PROTO_H__
#define PROTO_H__

#define RCVPORT "1989"                    //接受方ip地质放到argv指定，这里指定端口，1024以内端口yuliu, ”1989“，没有单位的数字没有意义，""，后面使用的时候使用atoi()


#define NAMESIZE    11              //用于处理不duiqi数据，防止掩盖bug
#define MTGROUP  "224.2.2.2"
struct msg_st
{   
    uint8_t name[NAMESIZE];             //uint解决类型长度问题
    uint32_t math;
    uint32_t chinese;
}__attribute__((packed));                 //告诉编译器buduiqi 







#endif
