#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/wait.h>

#define KEYPATH "/etc/services"
#define KEYPROJ 'a'

#define MEMSIZE 1024

/**共享内存：1.mmap，io存储映射（io/adv/mmap/）；2.shmget等函数**/


int main()
{
    int key;
    int shmid;
    pid_t pid;
    char *ptr;

    key = ftok(KEYPATH, KEYPROJ);                     //亲缘关系进程可以使用匿名IPC，不关心key是多少
    if(key < 0){
        perror("ftok()");
        exit(1);
    }
    // shmget(IPC_PRIVATE, MEMSIZEM, 0600);
    shmid = shmget(key, MEMSIZE, IPC_CREAT|0600);
    if(shmid < 0){
        perror("shmget()");
        exit(1);
    }

    pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }

    if(pid == 0){                  //child write
        ptr = shmat(shmid, NULL, 0);                //映射一个地质
        if(ptr == (void *)-1){
            perror("shmat()");
            exit(1);
        }

        strcpy(ptr, "Hello!");
        shmdt(ptr);                 //解除映射
        exit(0);

    }
    else{
                                //parent read
        wait(NULL);
        ptr = shmat(shmid, NULL, 0);
        if(ptr == (void *)-1){
            perror("shmat()");
            exit(1);
        }

        puts(ptr);
        shmdt(ptr);
        shmctl(shmid, IPC_RMID, NULL);
        exit(0);

    }



}
