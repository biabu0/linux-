#include<stdio.h>
#include<stdlib.h>
#include<shadow.h>
#include<unistd.h>
#include<string.h>
#include<crypt.h>


int main(int argc, char *argv[])
{
    struct spwd *shadowline;
    char *input_pass;
    char *crypted_pass;
    if(argc < 2){
        fprintf(stderr, "Usage...\n");
        exit(1);
    }
    //从shadow文件中获取文件名argv[1]的一行
    shadowline = getspnam(argv[1]);
    //从终端中获取输入的密码
    input_pass = getpass("PassWord:");
    //crypt:将输入的密码，按照一定的格式生成加密口令
    crypted_pass = crypt(input_pass, shadowline->sp_pwdp);
    //比对输入的密码与设计的密码是否一致。
    if(strcmp(shadowline->sp_pwdp, crypted_pass) == 0)
        puts("ok!");
    else
        puts("failed!");

    exit(0);
}
