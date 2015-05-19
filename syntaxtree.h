#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H
#define cmdlength_max 20
#define cmdnumber_max 10
typedef struct Node
{
    char data[cmdlength_max];
    struct Node *lchild,*rchild;
}Node;


void DeleteTree(Node *tree);
int GetTokens(char *input, char (*elem)[cmdlength_max], int *end);

Node *CreateTree(char (*elem)[cmdlength_max],int start, int end);

Node *CreateSyntaxTree(char *input);

#endif


