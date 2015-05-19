#include <stdio.h>

int main(int argc, char *argv[])
{
    if(argc<2)
    {
        char m[20];
        scanf("%s",m);
        printf("input:%s\n",m);
    }
    else
    {
        int i=1;
        for(;i<argc;i++)
            printf("%s",argv[i]);
    }
    return 0;
}
