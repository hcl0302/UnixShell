#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syntaxtree.h"


void DeleteTree(Node *tree)
{
    //删除树,释放内存
    if(tree!=NULL)
    {
        if(tree->lchild!=NULL) DeleteTree(tree->lchild);
        if(tree->rchild!=NULL) DeleteTree(tree->rchild);
        free(tree);
        tree=NULL;
    }
}

int GetTokens(char *input, char (*elem)[cmdlength_max], int *len)
{
    //把input中的命令与操作符分开,分别存到elem二维数组里,并记录长度len
    int input_len=strlen(input);
    if(input_len==0)
    {
        *len=0; return 0;
    }
    int start=0,end=0,i=0;
    for(;end<input_len;end++)
    {
        if(input[end]=='|'||input[end]=='<'||input[end]=='>')
        {
            strncpy(elem[i++],&input[start],end-start); //复制运算符前面的一部分
            start=end+1;
            elem[i++][0]=input[end];    //复制运算符本省
        }
    }
    //增加判断if(start!=end) 
    strncpy(elem[i],&input[start],end-start);
    *len=i;
    return 1;
}

Node *CreateTree(char (*elem)[cmdlength_max],int start, int end)
{
    //把elem中的命令和操作符存到二叉树中
    Node *tree;
    if(start==end)
        //最后剩下的是单独的命令
    {
        tree=(Node *)malloc(sizeof(Node));
        strcpy(tree->data,elem[start]);
        tree->lchild=NULL;
        tree->rchild=NULL;
        return tree;
    }
    int i,sep=-1;
    for(i=start;i<=end;i++)
    {
        //从本段命令中找出优先级最低的操作符,优先级顺序从低到高为|,<,>,
        if(elem[i][0]=='|')
            sep=i;
    }
    if(sep==-1)
    {
        for(i=start;i<=end;i++)
            if(elem[i][0]=='<'||elem[i][0]=='>')
                sep=i;
    }
    if(sep==-1)
    {
        printf("Unknown formular\n");
        return NULL;
    }
    tree=(Node *)malloc(sizeof(Node));
    if(!tree)
    {
        printf("malloc failed.\n");
        return NULL;
    }
    //以分隔符为根建立树,左右两侧分别为左右子树
    strcpy(tree->data,elem[sep]);
    //后面加上判断sep是不是首尾的位置
    tree->lchild=CreateTree(elem,start,sep-1);
    tree->rchild=CreateTree(elem,sep+1,end);
    return tree;
}

Node *CreateSyntaxTree(char *input)
{
    char elem[cmdnumber_max][cmdlength_max];
    int end;
    memset(elem,'\0',sizeof(elem));
    if(!GetTokens(input,elem,&end))
        return NULL;
    return CreateTree(elem,0,end);
}

