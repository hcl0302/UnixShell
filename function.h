#ifndef FUNCTION_H
#define FUNCTION_H

#include "syntaxtree.h"
#define jobnum_max 20
#define cmd_length 50
/*job管理系统定义*/

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

/*环境变量相关*/
void EchoPath();    //查看环境变量
void AddPath(char *);   //添加环境变量

/*job相关函数实现*/

void BackGround(int pid);   //后台运行
void ForeGround(char *p);    //前台运行
void StopPid(char *p);   //中止进程
void ContinuePid(char *p);  //继续进程
void ShowJobs(); //显示jobs里的后台进程

/*信号,管理后台程序*/
void sighandler_chld(int signum);
/*对SIGINT和SIGTSTP信号的处理*/
void sighandler_int(int signum);

/*输入输出重定向相关函数实现*/
int OutputRedirect(char *output);
int InputRedirect(char *input);

/*对输入命令的解析和处理*/
char **ResolveCmd(const char *cmd,int *argc);   //分解单条命令为argv[]
int Interpret(const char *cmd, int redirect,char *target,int bg);   
    //解析执行一条命令
int ExecTree(Node *tree, int bg);   //解析二叉树

/*内建命令结构体定义*/
typedef struct{  //定义内建命令,名称+函数指针
    const char *name;
    int (*handler)();
}CmdFmt;

//内建命令函数声明

extern int catHistory();//查看历史命令
int ChangeDir(char *path);  //改变工作目录
int Exit(); //退出




#endif
