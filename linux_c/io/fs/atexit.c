#include<stdio.h>
#include<stdlib.h>

void f1(void)
{

    puts("f1");
}

void f2(void)
{

    puts("f2");
}

int main()
{
   
/*Begin!
End!
f2
f1
*/

    puts("Begin!");

    atexit(f1);
    
    atexit(f2);


    puts("End!");



    exit(0);
}
