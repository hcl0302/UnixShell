#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <sys/utsname.h>
#include "filesystem.h"
#include "function.h"
#include "syntaxtree.h"

#define path_max 1024
#define cmd_length 50
#define cmd_option_nummax 50
#define envpath_count_max 15

JobManage jobmanager;   //管理后台任务

char currentpath[path_max]; //当前工作路径
char welcome_info[path_max];    //命令提示符信息
int welcome_len;    //命令提示符插入工作路径的位置
extern int run_pid; //前台运行进程的pid

/*环境变量*/
char envpath_file[path_max];    //存放环境变量的文件:userdir/.test_path
char envpath[envpath_count_max][100];
//={"/usr/local/sbin","/usr/local/bin","/usr/sbin","/usr/bin","/sbin","/bin","/usr/games","/usr/local/games"};
int envpath_num;
/*内建命令定义*/
CmdFmt command_list[10]={    //命令名+函数指针
    {"history",catHistory},{"cd",ChangeDir},{"exit",Exit},{"jobs",NULL},{"bg",NULL},{"fg",NULL},{"echopath",NULL},{"addpath",NULL},{NULL,NULL}
};

/*常用命令定义,用于空命令时Tab的提示*/
CmdFmt command_use[10]={
    {"eog",NULL},{"evince",NULL},{"dot",NULL},{"transmageddon",NULL},{"kazam",NULL},{NULL,NULL}
};

/*导入环境变量*/
int loadEnvpath()
{
    //如果不存在，先建立文件并写入初始的path（判断是否成功），然后导入
    int fd;
    if((fd=open(envpath_file,O_RDONLY,0))<0)   //如果文件不存在
    {
        if((fd=open(envpath_file,O_WRONLY|O_CREAT,0600))<0) return 0;    //创建文件
        char path0[8][30]={"/usr/local/sbin","/usr/local/bin","/usr/sbin","/usr/bin","/sbin","/bin","/usr/games","/usr/local/games"};
        int i;
        for(i=0;i<8;i++)
        {
            write(fd,path0[i],strlen(path0[i])); write(fd,";",1);
            strcpy(envpath[i],path0[i]);
        }
        envpath[i][0]='\0';
        envpath_num=8;
    }
    else    //文件已存在，导入环境变量
    {
        char buf[4096]; ssize_t size;
        size=read(fd,buf,4096);
        int i=0,j=0,cur=0;
        while(cur<size)
        {
            if(buf[cur]==';')
            {
                envpath[i++][j]='\0'; j=0;
            }
            else if(buf[cur]!=' '&&buf[cur]!='\n') envpath[i][j++]=buf[cur];
            cur++;
        }
        envpath[i][j]='\0';
        envpath_num=i;
    }
    close(fd);
    return 1;
}

int catHistory()
{
    //显示历史命令
    register HIST_ENTRY **the_list;
    register int i;
    the_list = history_list ();
    if (the_list)
        for (i = 0; the_list[i]; i++)
            printf ("%d: %s\n", i + history_base, the_list[i]->line);
    return 0;
}


char* getuser_dir(int uid)
{
    //得到运行该程序的客户的家目录
    FILE *fp;
    char buffer[path_max];
    sprintf(buffer,"mawk -F: '{if($3==%d){print  $6}}' /etc/passwd",uid);
    fp=popen(buffer,"r");
    fgets(buffer,strlen(buffer),fp);
    pclose(fp);
    return strdup(buffer);
}

/*填充该程序的命令表TAB功能*/
static char *command_generator(const char *text, int state)
{   
    const char *name;
    static int list_index,len,search_env,cmd_index,use_index;
    static char cmd_option[cmd_option_nummax][cmd_length];
    if(!state)
    {
      list_index=0;
      use_index=0;
      len = strlen(text);
      search_env=-1;
      cmd_index=0;
    }
    if(!len)
    {
        //提示常用命令
        while((name=command_use[use_index].name)!=NULL)
        {
            use_index++; return strdup(name);
        }
    }
    else 
    {
        //查找环境变量目录构建可用命令列表
        if(search_env<0)
        {
            int i=0;
            search_env=0;
            for(;i<envpath_num;i++)
                SearchCmdOption(envpath[i],text,cmd_option,&search_env);
        }
        while((name=command_list[list_index].name)!=NULL)
        {
          list_index++;
          if(strncmp(name,text,len) == 0)
          {
             return strdup(name);
          }
        }
        while(cmd_index<search_env)
        {
            name=cmd_option[cmd_index++];
            return strdup(name);
        }
    }
    return ((char *)NULL);
}

/*填充选项命令，该处暂忽略*/
static char *option_generator(const char *text, int state)
{
    //static int list_index,len;
    return NULL;
}

/*与readline库的命令填充接口*/

char **readline_command_completion(const char *text, int start, int end)
{
    char **matches = (char **)NULL;
    if(start == 0)
        matches = rl_completion_matches(text,command_generator);
    else
        matches = rl_completion_matches(text,option_generator);
    return matches;
}

/*readline初始化*/

static int readline_init(char *History)
{
    //strcpy(currentpath,"currentpath");
    char *userdir;
    rl_readline_name="UnixShell";
    rl_attempted_completion_function = readline_command_completion; //自动补全接口
    stifle_history(500);
    userdir = getuser_dir(getuid());    //获取用户家目录
    if(strlen(userdir)>0 && userdir[strlen(userdir)-1]=='\n')
        userdir[strlen(userdir)-1]='\0';
    strcpy(currentpath,userdir);
    sprintf(History,"%s/%s",userdir,".test_history");   //存储历史记录的文件位置
    sprintf(envpath_file,"%s/%s",userdir,".test_path"); //存储环境变量的文件位置
    free(userdir);
    read_history(History);  //读取历史记录
    /*设置快捷键*/
    //rl_bind_key('\t',key_stop);    
    return 0;  

}
 

/*store the commands into History file*/
static int readline_deinit(char *History)
{
    write_history(History);
    return 0;
}

 
/*
static int command_parse(char *cmdstr)
{
    const char *name;
    char *cmdname;
    int list_index=0;
    char cmd[path_max];
    strcpy(cmd,cmdstr);
    cmdname=strtok(cmd," ");
    if(cmdname==NULL)
        return -1;
    while((name=command_list[list_index].name)!=NULL)
    {
        if(!strcmp(name,cmdname))
        {
            printf("Find a command defined within program!\n");
            command_list[list_index].handler();
            return 1;
        }
        list_index++;
    }
    return 0;
}*/

/*安装信号，用于管理后台进程和处理信号*/
static void deal_signals()
{
    
    run_pid=0; //初始化全局变量,用于存储前台运行进程的pid
    if((signal(SIGCHLD, sighandler_chld) == SIG_ERR)
        ||(signal(SIGINT,sighandler_int))==SIG_ERR
        ||(signal(SIGTSTP,sighandler_int))==SIG_ERR)
    {  
        printf("signal error.\n");    
    }  
    /*
    sigset_t intmask;
    sigemptyset(&intmask);// 将信号集合设置为空 
    sigaddset(&intmask,SIGINT);// 加入中断 Ctrl+C 信号
    sigaddset(&intmask,SIGTSTP);// 加入中断 Ctrl+z 信号
    //阻塞信号
    sigprocmask(SIG_BLOCK,&intmask,NULL);*/
}

int main()
{
    char *cmd=NULL;
    Node *tree=NULL;
    jobmanager.jobnum=0;
    char History[path_max]; // history_dir/history_file，记录历史命令的文件地址

    
    deal_signals(); //安装和屏蔽信号

    /*初始化readline*/
    
    char *username=getlogin();
    struct utsname uts;
    if(uname(&uts)<0)
    {
        printf("无法获得主机信息\n");
    }
    readline_init(History);

    /*导入环境变量*/
    if(!loadEnvpath()) printf("从路径'%s'导入环境变量失败！无法创建新文件存储环境变量\n",envpath_file);;
    
    /*准备命令提示符*/
    sprintf(welcome_info,"%s@%s:",username,uts.sysname);
    welcome_len=strlen(welcome_info);

    /*初始化工作目录*/
    ChangeDir(currentpath);

    /*接收和处理命令*/
    do
    {
        if(cmd!=NULL)
        {
            free(cmd); cmd=NULL;
        }
        DeleteTree(tree);
        cmd=readline(welcome_info);
        add_history(cmd);
        tree=CreateSyntaxTree(cmd);
    }while(ExecTree(tree,0));
    readline_deinit(History);
    return 0;
}
