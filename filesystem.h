#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/types.h>
#define cmd_length 30
#define cmd_option_nummax 50

typedef struct dirnode
{
    //用来存储访问过的文件夹的链表，避免文件树带环的情况
    ino_t inode; //每个文件夹的inode号
    struct dirnode *next;
}dirnode;

dirnode *dirlist;

dirnode *createlist(dirnode *head);
    //创建存储文件夹inode号的链表

void insertdir(dirnode *head,ino_t inode);
    //插入新节点，存储新文件夹的inode

int traverse(dirnode *head, ino_t inode);
    //用于判断文件树成环的情况，遍历链表，查看链表中有无与该inode重复的文件夹Inode号，有的话说明已经访问过该文件夹，返回1

int myftw(char *path, const char *file, char *filepath);

int SearchFile(const char *envpath, const char *file, char *filepath);

/*遍历目录查找备选命令*/
int fillcmdop(char *path,const char *text, char cmd_option[cmd_option_nummax][cmd_length], int option_num);

/*根据关键字查找备选命令*/
int SearchCmdOption(const char *envpath, const char *text, char cmd_option[cmd_option_nummax][cmd_length], int option_num);

#endif
