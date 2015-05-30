#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>
#include "filesystem.h"
#include "function.h"
#include "syntaxtree.h"

#define envpath_count 10
#define argv_num 3
#define argv_length 20
#define cmd_length 30
#define jobnum_max 10
const char envpath[envpath_count][30]={"/usr/local/sbin","/usr/local/bin","/usr/sbin","/usr/bin","/sbin","/bin","/usr/games","/usr/local/games"};

typedef struct Job
{
    int status; //1表示正常运行,0表示退出,2表示停止,-1表示移到前台运行
    int pid;
    char cmd[cmd_length]; //指向argv
}Job;

typedef struct JobManage
{
    Job jobs[jobnum_max];   //存放Pid,cmd,状态信息
    int jobnum;
}JobManage;

JobManage jobmanager;

void BackGround(int pid)  
{
    if(pid>0)
    {
    kill(pid,SIGCONT);
     //pid_jobs[jobs_num--]=0;
     //不要忘记对jobs进行更改
    }
}

void ForeGround(char *p)
{
    if(p!=NULL)
    {
        int jobnum=atoi(p);
        int pid=jobmanager.jobs[jobnum-1].pid;
        //从Jobs列表删除
        jobmanager.jobs[jobnum-1].status=-1;
        printf("%s",jobmanager.jobs[jobnum-1].cmd);
        //进程移到前台
        kill(pid,SIGSTOP);
        kill(pid,SIGCONT);
        waitpid(pid,NULL,0);
    }
}    

void StopPid(char *p)
{
    if(p!=NULL)
    {
        int jobnum=atoi(p);
        int pid=jobmanager.jobs[jobnum-1].pid;
        kill(pid,SIGSTOP);
    }
}

void ContinuePid(char *p)
{
    if(p!=NULL)
    {
        int jobnum=atoi(p);
        int pid=jobmanager.jobs[jobnum-1].pid;
        kill(pid,SIGCONT);
    }
}

void ShowJobs()
{
    int i=0;
    while(i<jobmanager.jobnum && jobmanager.jobs[i].status>0)
    {
        printf("[%d]  ",i+1);
        switch(jobmanager.jobs[i].status)
        {
            case 1:
                printf("运行中  ");
                break;
            case 2:
                printf("已停止  ");
                break;
        }
        printf("%s\n",jobmanager.jobs[i].cmd);
        i++;
    }
}

char **ResolveCmd(const char *cmd,int *argc)
{
    int i=0,j=0,k=0,cmd_count=1;
    for(;i<strlen(cmd);i++)
    {
        if(cmd[i]==' '&&i+1<strlen(cmd))
            cmd_count++;
    }
    char **argv;
    argv=(char **)malloc((cmd_count+1)*sizeof(char *)); //多分配一个用来表示结尾
    for(i=0;i<cmd_count;i++)
        argv[i]=(char *)malloc(argv_length*sizeof(char));
    argv[i]=NULL;   //标识结尾
    for(i=0;i<strlen(cmd);i++)
    {
        if(cmd[i]!=' ')
            argv[j][k++]=cmd[i];
        else
        {
            //下一个argv
            argv[j][k]='\0';
            j++;k=0;
        }
    }
    //无空格结束也要加终止符号
    argv[j][k]='\0';
    *argc=cmd_count;
    return argv;
}

int Interpret(const char *cmd, int redirect,char *target,int bg)
{
    //cmd中包含两部分,一是命令执行文件本省,第二是argv[]
    int argc;
    char **argv=ResolveCmd(cmd,&argc);
    if(!strcmp(argv[0],"cd"))
        printf("This is an inner cmd.\n");
    else if(!strcmp(argv[0],"jobs"))
    {
        ShowJobs();
    }
    else if(!strcmp(argv[0],"fg"))
    {
        //前台运行
        ForeGround(argv[1]);
    }
    else if(!strcmp(argv[0],"stop"))
        StopPid(argv[1]);
    else if(!strcmp(argv[0],"continue"))
        ContinuePid(argv[1]);
    else if(!strcmp(argv[0],"q"))
        return 0;
    else
    {
        char filepath[PATH_MAX+1]={'\0'};
        int i=0,find=0;
        while(!find && i<envpath_count)
        {
            //依次从各个环境变量的路径中寻找
            find=SearchFile(envpath[i++],argv[0],filepath);
        }
        if(find && bg==0)
        {
            int pid=fork();
            if(pid>0)
                waitpid(pid,NULL,0);
            else
            {
                if(redirect==1) OutputRedirect(target);
                else if(redirect==0) InputRedirect(target);
                execv(filepath,argv);
                exit(0);
            }
           
        }
        else if(find && bg==1)
        {
            //后台运行
            int pid=fork();
            if(pid>0)
            {
                //父进程把后台进程添加到job管理系统中
                int pos=0;  //找空位置,优先填满前面被删掉的位置
                while(pos<jobmanager.jobnum && jobmanager.jobs[pos].status>0)
                    pos++;
                //结果是pos占到了jobnum之前的被删位置,或者是pos==jobnum
                jobmanager.jobs[pos].pid=pid;
                int i=0;
                for(;i<strlen(cmd);i++)
                    jobmanager.jobs[pos].cmd[i]=cmd[i]; //保存cmd信息
                jobmanager.jobs[pos].cmd[i++]='&'; //保存cmd信息
                jobmanager.jobs[pos].cmd[i]='\0'; //结尾
                jobmanager.jobs[pos].status=1;   //正常运行 
                if(pos==jobmanager.jobnum) {
                    jobmanager.jobnum++;    //只有pos开辟了新的坑位时,才占位
                    if(jobmanager.jobnum>jobnum_max)
                        printf("jobs中的后台程序即将达到上限\n");
                }
                printf("[%d] %d",pos+1,pid);
            }
            else
            {
                //子进程负责后台执行
                if(redirect==1) OutputRedirect(target);
                else if(redirect==0) InputRedirect(target);
                execv(filepath,argv);
                exit(0);
                //有个问题:子进程退了之后,父进程怎么查询?会出现相同pid的情况吗?貌似不会
            }
        }
        else printf("Unknwon cmd.\n");
    }
     //程序结束后要清理argv
    int i=0;
    for(;i<argc;i++)
    {
        free(argv[i]);
    }
    free(argv);

    return 1;
}


int ExecTree(Node *tree, int bg)
{
    if(tree==NULL) return 1;
    if(tree->lchild==NULL && tree->rchild==NULL)
    {
        return Interpret(tree->data,-1,NULL,bg);
    }
    if(tree->data[0]=='&')
    {
        if(ExecTree(tree->lchild,1) && ExecTree(tree->rchild,0))
            return 1;
        else return 0;
    }
    if(tree->data[0]=='>')
    {
        return Interpret(tree->lchild->data,1,tree->rchild->data,bg);
    }
    else if(tree->data[0]=='<')
    {
        return Interpret(tree->lchild->data,0,tree->rchild->data,bg);
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
            ExecTree(tree->rchild,bg);
            waitpid(pid,NULL,0);
            dup2(sfd,STDIN_FILENO); //还原标准输入
        }
        else    //子进程
        {
            dup2(fds[1],STDOUT_FILENO); //管道重定向到标准输出
            close(fds[0]); close(fds[1]);
            ExecTree(tree->lchild,bg); //这里的bg情况暂时搞不清楚
            exit(0);
        }
    }
    return 1;
    
}

static void sig_handler(int signum)  
{
    int pid,status,i=0;
    while(i<jobmanager.jobnum && (pid=waitpid(jobmanager.jobs[i].pid,&status,WNOHANG|WUNTRACED|WCONTINUED))<=0)
        i++;    //从jobs任务管理中寻找是否有后台的pid进程可回收
    if(i==jobmanager.jobnum)
    {
        printf("A foreground thread has exited.\n");
        return;
    }
    if(pid<0)  
    {  
        printf("waitpid error after signal\n");  
    }
    else if(pid>0 && WIFSTOPPED(status))
    {
        printf("child [%d] is stoped.\n",i+1);
        jobmanager.jobs[i].status=2;
    }
    else if(pid>0 && (WIFEXITED(status)||WIFSIGNALED(status)))
    {
        //printf("child is exit.\n");
        jobmanager.jobs[i].status=0;
        printf("[%d] 已完成 %s\n",i+1,jobmanager.jobs[i].cmd);
    }
    else
    {
        printf("child [%d] is continued.\n",i+1);
        jobmanager.jobs[i].status=1;
    }
    /*
    if(WIFEXITED(status))
        printf("child exit normally\n");
    else if(WIFSIGNALED(status))
        printf("child exit abnormally\n");
    else if(WIFSTOPPED(status))
        printf("child is stopped\n");
        */
        
}  

int main()
{
    char cmd[50]={'\0'};
    Node *tree=NULL;
    jobmanager.jobnum=0;
    //安装信号,用于管理后台进程
    if(signal(SIGCHLD, sig_handler) == SIG_ERR)  
    {  
        printf("signal error.\n");  
        return 0;  
    }  
    do{
        DeleteTree(tree);
        fgets(cmd,sizeof(cmd),stdin);
        cmd[strlen(cmd)-1]='\0';
        tree=CreateSyntaxTree(cmd);
    }while(ExecTree(tree,0));
    
    return 0;
}
