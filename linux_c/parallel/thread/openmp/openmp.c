#include<stdio.h>
#include<stdlib.h>


int main()
{
#pragma omp parallel
{
    puts("hello");
    puts("world!");

}

    exit(0);
}
