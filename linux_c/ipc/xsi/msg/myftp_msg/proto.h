#ifndef PROTO_H__
#define PROTO_H__

#define KEYPATH "/etc/services"
#define KEYPROJ 'a'

#define PATHMAX 1024
#define DATAMAX 1024

typedef enum msg_type_e
{
    MSG_PATH = 1,
    MSG_DATA,
    MSG_EOT,
}msg_type_t;


typedef struct msg_path_st
{
    char path[PATHMAX];
}msg_path_t;

typedef struct msg_c2s_st
{
    long mtype;
    msg_path_t msg_path;
}msg_c2s_t;

typedef struct msg_data_st
{
    unsigned len;
    char data[DATAMAX];
}msg_data_t;


typedef struct msg_eot_st
{

}msg_eot_t;

typedef struct msg_s2c_st
{
    long mtype;
    union {
        msg_data_t msg_data;
        msg_eot_t msg_eot;
    };
}msg_s2c_t;




#endif
















