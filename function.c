#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "function.h"
int OutputRedirect(char *output)
{
    int fd;
    if((fd=open(output,O_WRONLY|O_CREAT|O_TRUNC))==-1)
    {
        printf("Cannot open.\n");
        return 0;
    }
    dup2(fd,STDOUT_FILENO); //输出重定向到文件
    return 1;
}

int InputRedirect(char *input)
{
    int fd;
    if((fd=open(input,O_RDONLY))==-1)
    {
        printf("没有名为%s的文件或目录\n",input);
        return 0;
    }
    dup2(fd,STDIN_FILENO); //输入重定向到文件
    return 1;
}

