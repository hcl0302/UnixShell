#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include "filesystem.h"
#include "function.h"
#include "syntaxtree.h"

const char *envpath=".";

int Interpret(const char *cmd, int redirect,char *target)
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
                if(redirect==1) OutputRedirect(target);
                else if(redirect==0) InputRedirect(target);
                execle(filepath,cmd,(char *)0,(char *)0);
                exit(0);
            }
        }
        else printf("Unknwon cmd.\n");
    }
    return 1;
}


int ExecTree(Node *tree)
{
    if(tree==NULL) return 1;
    if(tree->lchild==NULL && tree->rchild==NULL)
    {
        return Interpret(tree->data,0,NULL);
    }
    if(tree->data[0]=='>')
    {
        return Interpret(tree->lchild->data,1,tree->rchild->data);
    }
    else if(tree->data[0]=='<')
    {
        return Interpret(tree->lchild->data,0,tree->rchild->data);
    }
    else if(tree->data[0]=='|')
    {
        int fds[2],pid;
        pipe(fds);
        pid=fork();
        if(pid>0) //父进程
        {
            int sfd=dup(STDIN_FILENO);  //先把标准输入保存下来
            dup2(fds[0],STDIN_FILENO);  //管道重定向到标准输入
            close(fds[1]);  close(fds[0]);
            ExecTree(tree->rchild);
            wait(NULL);
            dup2(sfd,STDIN_FILENO); //还原标准输入
        }
        else    //子进程
        {
            dup2(fds[1],STDOUT_FILENO); //管道重定向到标准输出
            close(fds[0]); close(fds[1]);
            ExecTree(tree->lchild);
            exit(0);
        }
    }
    return 1;
    
}

int main()
{
    char cmd[50]={'\0'};
    Node *tree=NULL;
    do{
        DeleteTree(tree);
        fgets(cmd,sizeof(cmd),stdin);
        cmd[strlen(cmd)-1]='\0';
        tree=CreateSyntaxTree(cmd);
    }while(ExecTree(tree));
    
    return 0;
}
