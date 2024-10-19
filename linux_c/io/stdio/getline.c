#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main(int argc, char **argv)
{
    FILE *fp;
    char *buf = NULL;
    size_t bufsize;

    if(argc < 2){
        fprintf(stderr,"Usage...\n");//行刷新
        exit(1);
    }

    fp = fopen(argv[1],"r");
    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }
    /*   !!!!!!!!!!           */
    buf = NULL;
    bufsize = 0

    while(1)
    {
        if(getline(&buf, &bufsize, fp) < 0)
            break;
        printf("%ld\n",strlen(buf));


    }

    fclose(fp);
    exit(0);
}
