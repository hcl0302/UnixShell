#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int Interpret(char *cmd)
{
    if(!strcmp(cmd,"cd"))
        printf("This is an inner cmd.\n");
    else if(!strcmp(cmd,"git"))
    {
        int pid=fork();
        if(pid>0)   //父进程
            wait(NULL);
        else
        {
            execle("outcmd","outcmd",(char *)0,(char *)0);
            exit(0);
        }
    }
    else if(!strcmp(cmd,"q"))
        return 0;
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
