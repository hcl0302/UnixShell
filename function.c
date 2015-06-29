#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>
#include "filesystem.h"
#include "function.h"
#include "navigation.h"

#define envpath_count_max 15
#define argv_num 10
#define argv_length 100
#define path_max 1024

extern char currentpath[path_max];  //当前工作目录
extern char welcome_info[path_max]; //命令提示符
extern int welcome_len; //命令提示符插入工作路径位置
extern char envpath[envpath_count_max][100];    //环境变量
extern char envpath_file[path_max]; //环境变量位置
extern int envpath_num; //环境变量数目
//const char envpath[envpath_count_max][30]={"/usr/local/sbin","/usr/local/bin","/usr/sbin","/usr/bin","/sbin","/bin","/usr/games","/usr/local/games"};

extern JobManage jobmanager;    //管理后台
int run_pid;    //记录前台运行的子进程pid
char run_cmd[cmd_length];    //记录前台运行的子进程cmd

/*环境变量相关*/
void EchoPath() //查看环境变量
{
    int i=0;
    for(;i<envpath_num;i++)
        printf("%s\n",envpath[i]);
}

void AddPath(char *newpath)  //添加环境变量
{
    int len=strlen(newpath);
    if(len==0) return;
    strcpy(envpath[envpath_num],newpath);
    envpath_num++; envpath[envpath_num][0]='\0';
    //写入文件
    int fd;
    if((fd=open(envpath_file,O_WRONLY,0600))<0)
    {
        printf("无法打开环境变量文件%s\n",envpath_file); return;
    }
    lseek(fd,0,SEEK_END);
    write(fd,newpath,len);
    write(fd,";",1);
    close(fd);
}

/*job相关函数实现*/

void ForeGround(char *p)    //前台运行
{
    if(p!=NULL)
    {
        int jobnum=atoi(p);
        if(jobnum<1 || jobnum>jobmanager.jobnum || jobmanager.jobs[jobnum-1].status<=0)
        {
            printf("任务 %s 不在Jobs管理系统中\n",p); return;
        }
        int pid=jobmanager.jobs[jobnum-1].pid;
        run_pid=pid;
        strcpy(run_cmd,jobmanager.jobs[jobnum-1].cmd);
        //从Jobs列表删除
        jobmanager.jobs[jobnum-1].status=-1;
        printf("%s\n",jobmanager.jobs[jobnum-1].cmd);
        //进程移到前台
        //kill(pid,SIGSTOP);
        //kill(pid,SIGCONT);
        waitpid(pid,NULL,WUNTRACED);
        run_pid=0; run_cmd[0]='\0';
    }
}    

void StopPid(char *p)   //中止进程
{
    if(p!=NULL)
    {
        int jobnum=atoi(p);
        int pid=jobmanager.jobs[jobnum-1].pid;
        kill(pid,SIGSTOP);
    }
}

void ContinuePid(char *p)   //后台继续进程
{
    if(p!=NULL)
    {
        int jobnum=atoi(p);
        if(jobnum<1 || jobnum>jobmanager.jobnum || jobmanager.jobs[jobnum-1].status<=0)
        {
            printf("任务 %s 不在Jobs管理系统中\n",p); return;
        }
        int pid=jobmanager.jobs[jobnum-1].pid;
        kill(pid,SIGCONT);
    }
}

void ShowJobs() //显示jobs里的后台进程
{
    int i=0;
    while(i<jobmanager.jobnum && jobmanager.jobs[i].status>0)
    {
        printf("[%d] %d ",i+1,jobmanager.jobs[i].pid);
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

/*信号,管理后台程序*/

void sighandler_chld(int signum)  
{
    int pid,status,i=0;
    while(i<jobmanager.jobnum && (pid=waitpid(jobmanager.jobs[i].pid,&status,WNOHANG|WUNTRACED|WCONTINUED))<=0)
        i++;    //从jobs任务管理中寻找是否有后台的pid进程可回收
    if(i==jobmanager.jobnum)
    {
        //printf("A foreground thread has exited.\n");
        return;
    }
    if(pid<0)  
    {  
        printf("waitpid error after signal\n");  
    }
    else if(pid>0 && WIFSTOPPED(status))
    {
        printf("Job [%d] is stoped.\n",i+1);
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
        printf("Job [%d] is continued.\n",i+1);
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

/*对SIGINT和SIGTSTP信号的处理*/
void sighandler_int(int signum)
{
    //给前台运行的子进程发送SIGkill和sigstop信号
    //printf("signum:%d, run_pid:%d\n",signum,run_pid);
    if(run_pid)
    {
        switch(signum)
        {
            case SIGINT:
                kill(run_pid,SIGKILL);
                break;
            case SIGTSTP:
            {
                kill(run_pid,SIGSTOP);
                //父进程把后台进程添加到job管理系统中
                int pos,i;
                pos=0;  //找空位置,优先填满前面被删掉的位置
                while(pos<jobmanager.jobnum && jobmanager.jobs[pos].status>0)  pos++;
                //结果是pos占到了jobnum之前的被删位置,或者是pos==jobnum
                jobmanager.jobs[pos].pid=run_pid;
                for(i=0;i<strlen(run_cmd);i++)
                    jobmanager.jobs[pos].cmd[i]=run_cmd[i]; //保存cmd信息
                jobmanager.jobs[pos].cmd[i]='\0'; //结尾
                jobmanager.jobs[pos].status=2;   //停止状态
                if(pos==jobmanager.jobnum) {
                    jobmanager.jobnum++;    //只有pos开辟了新的坑位时,才占位
                    if(jobmanager.jobnum>jobnum_max)
                        printf("jobs中的后台程序即将达到上限\n");
                }
                printf("[%d] %d 已停止  %s\n",pos+1,run_pid,run_cmd);
            }
                break;
            default:break;
        }
    }
}

/*屏蔽信号*/
static void mask_signals()
{
    sigset_t intmask;
    sigemptyset(&intmask);// 将信号集合设置为空 
    sigaddset(&intmask,SIGINT);// 加入中断 Ctrl+C 信号
    sigaddset(&intmask,SIGTSTP);// 加入中断 Ctrl+z 信号
    //阻塞信号
    sigprocmask(SIG_BLOCK,&intmask,NULL);
}

/*解除屏蔽Ctrl+C信号处理
static void unmask_signals()
{
    sigset_t intmask;
    sigemptyset(&intmask);// 将信号集合设置为空 
    sigaddset(&intmask,SIGINT);// 加入中断 Ctrl+C 信号
    //阻塞信号
    sigprocmask(SIG_UNBLOCK,&intmask,NULL);
}*/

/*输入输出重定向相关函数实现*/

int OutputRedirect(char *output)
{
    //输出重定向
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
    //输入重定向
    int fd;
    if((fd=open(input,O_RDONLY))==-1)
    {
        printf("没有名为%s的文件或目录\n",input);
        return 0;
    }
    dup2(fd,STDIN_FILENO); //输入重定向到文件
    return 1;
}

/*对输入命令的解析和处理*/

char **ResolveCmd(const char *cmd,int *argc)
{
    //分解单条命令为argv[]
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
    if(j<cmd_count) argv[j][k]='\0';
    *argc=cmd_count;
    return argv;
}

int Interpret(const char *cmd, int redirect,char *target,int bg)
{
    //解析执行一条命令
    //cmd中包含两部分,一是命令执行文件本省,第二是argv[]
    if(strlen(cmd)==0) return 1;
    int argc;
    char **argv=ResolveCmd(cmd,&argc);
    if(!strcmp(argv[0],"cd"))
    {
        if(argc>1) ChangeDir(argv[1]);
        else    //开启文件导航系统
        {
            char finalpath[path_max];
            Navigation(finalpath);
            ChangeDir(finalpath);
        }
    }
    else if(!strcmp(argv[0],"jobs"))
    {
        ShowJobs();
    }
    else if(!strcmp(argv[0],"fg"))
    {   //前台运行
        ForeGround(argv[1]);
    }
    else if(!strcmp(argv[0],"stop"))
        StopPid(argv[1]);
    else if(!strcmp(argv[0],"continue"))
        ContinuePid(argv[1]);
    else if(!strcmp(argv[0],"bg"))  //后台继续运行
        ContinuePid(argv[1]);
    else if(!strcmp(argv[0],"history"))
        catHistory();
    else if(!strcmp(argv[0],"echopath"))
        EchoPath();
    else if(!strcmp(argv[0],"addpath"))
        AddPath(argv[1]);
    else if(!strcmp(argv[0],"exit"))
        return Exit();
    else
    {
        int i=0,find=0;
        char filepath[PATH_MAX+1]={'\0'};
        find=isEXE(argv[0]);    //输入的命令是否带'/'，可能指向EXEfile
        if(find)
            strcpy(filepath,argv[0]);
        else    //从环境变量的目录中搜索
        {
            while(!find && i<envpath_num)
            {
                //依次从各个环境变量的路径中寻找
                find=SearchFile(envpath[i++],argv[0],filepath);
            }
        }
        if(find && bg==0)   //找到命令,前台运行
        {
            int pid=fork(); run_pid=pid; strcpy(run_cmd,cmd);
            if(pid>0) {
                waitpid(pid,NULL,WUNTRACED); run_pid=0; run_cmd[0]='\0';
            }
            else
            {
                if(redirect==1) OutputRedirect(target);
                else if(redirect==0) InputRedirect(target);
                mask_signals();     //子进程屏蔽信号
                if(execv(filepath,argv)==-1)
                    printf("不能打开指定文件\n");
                exit(0);
            }
           
        }
        else if(find && bg==1)  //找到命令,后台运行
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
                printf("[%d] %d\n",pos+1,pid);
            }
            else
            {
                //子进程负责后台执行
                if(redirect==1) OutputRedirect(target);
                else if(redirect==0) InputRedirect(target);
                mask_signals();     //信号屏蔽
                if(execv(filepath,argv)==-1)
                    printf("不能打开指定文件\n");
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
    //解析二叉树
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

int ChangeDir(char *path)
{
    //改变工作目录
    if(!chdir(path))
    {
        if(getcwd(currentpath,sizeof(currentpath))==NULL)
        {
            printf("重新获取工作目录失败\n"); return 0;
        }
        sprintf(&welcome_info[welcome_len],"%s$ ",currentpath);
        return 1;
    }
    printf("改变工作目录失败\n");
    return 0;
}

int Exit()
{
    //退出
    return 0;
}
