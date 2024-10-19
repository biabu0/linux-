#include<stdlib.h>
#include<stdio.h>
#include<glob.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define DELIM       " \n\t"

struct cmd_st{                //便于后续开发（判断是内部命令还是外部命令）

    glob_t globres;
};                              //;结尾


static void prompt(void)
{
    printf("mysh-0.1$ ");
}

static void parse(char *line, struct cmd_st *res)
{
    /***********把命令行解析出来，传回主调函数*****************/
    char *tok;
    int i = 0;
    
    while(1){
        tok = strsep(&line, DELIM);             //通过此函数将一行数据按照一定格式分割
        if(tok == NULL)
            break;
        if(tok[0] == '\0')                      //多个空格连续，跳出此次循环
            continue;
        /*****************************
        
        要将每次分割出来的数据存储起来，组成一个数组存放，考虑使用
        NOCHECK没有匹配项就将tok存入，满足需求；在第此一存放的时候不能使用添加，不知道里面已经有了什么东西。
        
        ********************/
        glob(tok, GLOB_NOCHECK | GLOB_APPEND * i, NULL, &res->globres);

        i = 1;

    }


}


int main()
{
    char *linebuf = NULL;
    size_t linebuf_size = 0;           //getline函数需要做这两个步骤
    struct cmd_st cmd;
    pid_t pid;
    while(1){
        prompt();    //打印提示符

        if(getline(&linebuf, &linebuf_size, stdin) < 0){
            perror("getline()");
            break;
        }      //获取输入

        parse(linebuf, &cmd);     //解析

        if(0){               //内部命令
            //
        }

        else{               //外部命令
        
            pid = fork();
            if(pid < 0){
                perror("fork()");
                exit(1);
            }
            else if(pid == 0){
                execvp(cmd.globres.gl_pathv[0], cmd.globres.gl_pathv);
                perror("execvp()");
                exit(1);
                
            }
            else{
                wait(NULL);
            }

        }


    }
    exit(0);
}
