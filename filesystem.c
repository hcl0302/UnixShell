#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h> //for PATH_MAX

#include "filesystem.h"

//任务1：在指定路径下搜寻可执行文件，没有返回NULL,有则返回地址指针



dirnode *createlist(dirnode *head)
{
    //创建存储文件夹inode号的链表
    head=(dirnode *)malloc(sizeof(dirnode));
    if(head==NULL)
    {
        printf("malloc failed.\n");
        exit(1);
    }
    head->next=NULL;
    return head;
}

void insertdir(dirnode *head,ino_t inode)
{
    //插入新节点，存储新文件夹的inode
    dirnode *dir;
    dir=(dirnode *)malloc(sizeof(dirnode));
    if(dir==NULL)
    {
        printf("malloc failed.\n");
        exit(1);
    }
    dir->inode=inode;
    dir->next=head->next;
    head->next=dir;
}


int traverse(dirnode *head, ino_t inode)
{
    //用于判断文件树成环的情况，遍历链表，查看链表中有无与该inode重复的文件夹Inode号，有的话说明已经访问过该文件夹，返回1
    dirnode *dir;
    dir=head->next;
    while(dir!=NULL)
    {
        if(dir->inode==inode) return 1;
        dir=dir->next;
    }
    return 0;
}


int myftw(char *path, const char *file, char *filepath)
{
    struct stat path_stat;
    struct dirent *dirp;
    DIR *dp;
    int n;
    if(stat(path,&path_stat)!=0)
    {
        printf("stat failed.\n");
        return 0;
    }
    if(S_ISDIR(path_stat.st_mode)==0)   //路径是文件，判断文件名是否与目标文件相同
    {
        char name[255]={'\0'}; int i=0;
        for(n=strlen(path)-1;n>=0;n--)
            if(path[n]=='/') break;
        for(n+=1;n<strlen(path);n++) name[i++]=path[n];
        if(!strcmp(name,file)) 
        {
            strcpy(filepath,path);
            return 1;
        }
        return 0;
    }
    else //path正确是一个目录
    {
        if(traverse(dirlist,path_stat.st_ino))   //没访问过这个文件夹，避免成环
            return 0;
        insertdir(dirlist,path_stat.st_ino); //存入inode号
        n=strlen(path);
        path[n++]='/';
        path[n]=0;
        if((dp=opendir(path))==NULL)    //能否打开
            return 0;
        while((dirp=readdir(dp))!=NULL)  //遍历文件夹下的每一个文件或文件夹，调用函数myftw()
        {
            if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)  //跳过.和..这两个软链接
                continue;
            strcpy(&path[n],dirp->d_name);
            if(myftw(path,file,filepath)==1)
            {
                closedir(dp);
                return 1;
            }
        }
        closedir(dp); //关闭文件夹
    }
    return 0;
}

int SearchFile(const char *envpath, const char *file, char *filepath)
{
    dirlist=createlist(dirlist);
    char path[PATH_MAX+1]={'\0'};
    strcpy(path,envpath);
    return myftw(path, file,filepath);
}
