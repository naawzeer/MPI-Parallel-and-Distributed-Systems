#include <stdio.h>
#include <stdlib.h>

struct Node {
    int data;
    struct Node* left;
    struct Node* right;
};

struct Node* createNode(int value){
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));
    if(!newNode){
        printf("Memory Error\n");
        return NULL;
    }
    newNode ->data = value;
    newNode ->left = NULL;
    newNode ->right = NULL;
    return newNode;
}

int main() {
    struct Node* root = createNode(1);
    root ->left = createNode(2);
    root ->right = createNode(3);

    printf("Root: %d\n", root->data);
    printf("Left Child: %d\n", root->left->data);
    printf("Right Child: %d\n", root->right->data);

    return 0;
}