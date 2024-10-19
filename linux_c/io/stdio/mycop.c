#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

int main(int argc, char **argv)
{
    int ch;
    FILE *fps,*fpd;

    if(argc < 3)
    {
        fprintf(stderr,"Usage:%s <src_file> <dest_file>\n",argv[0]);
        exit(1);
    }
    fps = fopen(argv[1],"r");
    if(fps == NULL)
    {
        perror("fopen()");
        exit(1);
    }
    fpd = fopen(argv[2], "w");
    if(fpd == NULL)
    {
        fclose(fps);//fps已经打开成功，在此处关闭fps，不然造成内存泄漏
        perror("fopen()");
        exit(1);
    }
    while(1)
    {
        ch = fgetc(fps);
        if(ch == EOF)
            break;
        fputc(ch, fpd);

    }

    fclose(fps);
    fclose(fpd);
    
    exit(0);
}
