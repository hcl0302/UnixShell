#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include "filesystem.h"

const char *envpath="/bin";

int Interpret(const char *cmd)
{
    if(!strcmp(cmd,"cd"))
        printf("This is an inner cmd.\n");
    else if(!strcmp(cmd,"q"))
        return 0;
    else
    {
        char filepath[PATH_MAX+1]={'\0'};
        if(SearchFile(envpath,cmd,filepath))
        {
            int pid=fork();
            if(pid>0)
                wait(NULL);
            else
            {
                execle(filepath,cmd,(char *)0,(char *)0);
                exit(0);
            }
        }
        else printf("Unknwon cmd.\n");
    }
    return 1;
}

int main()
{
    char cmd[50]={'\0'};
    do{
        fgets(cmd,sizeof(cmd),stdin);
        cmd[strlen(cmd)-1]='\0';
    }while(Interpret(cmd));
    
    return 0;
}
