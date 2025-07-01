#include <stdio.h>
#include <stdlib.h>

typedef struct treenode {
    int value;
    struct treenode *left;
    struct treenode *right;
} treenode;

treenode *createnode(int value){
    treenode* result = malloc(sizeof(treenode));
    if (result != NULL){
        result->left = NULL;
        result->right = NULL;
        result->value = value;
    }
    return result;
}
void printtree(treenode *root){
    if(root == NULL){
        printf("---<empty---\n");
        return;
    }
    printf("value = %d\n", root->value);
    printf("left\n");
    printtree(root->left);
    printf("right\n");
    printtree(root->right);
    printf("Done\n");

}

int main(){
    treenode *n1 = createnode(10);
    treenode *n2 = createnode(50);
    treenode *n3 = createnode(30);
    treenode *n4 = createnode(20);
    treenode *n5 = createnode(15);
    treenode *n6 = createnode(12);

    n5->left = n6;
    n5->right = n4;
    n4->left = n3;
    n4->right = n2;
    n6->left = n1;

    printtree(n5);

    free(n1);
    free(n2);
    free(n3);
    free(n4);
    free(n5);
    free(n6);
    
}