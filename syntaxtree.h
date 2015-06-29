#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H
#define cmdlength_max 50
#define cmdnumber_max 10

/*定义存储命令的二叉树类型*/
typedef struct Node
{
    char data[cmdlength_max];
    struct Node *lchild,*rchild;
}Node;

/*销毁一颗二叉树*/
void DeleteTree(Node *tree);

/*把字符串按分隔符'|' '>' '<' '&'分割*/
int GetTokens(char *input, char elem[cmdnumber_max][cmdlength_max], int *end);

/*字符串片段转化为一颗二叉树*/
Node *CreateTree(char elem[cmdnumber_max][cmdlength_max],int start, int end);

/*从最初的字符串到最后的二叉树*/
Node *CreateSyntaxTree(char *input);

#endif


