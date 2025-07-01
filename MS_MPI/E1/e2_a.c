#include <stdio.h>
#include <stdlib.h>

typedef struct NODE Node;
struct NODE {
    int key;
    Node* smaller_keys;
    Node* larger_keys;
};

typedef struct {
    Node* root;
} Tree;

// Function prototypes
Tree* tree_create();
void tree_insert_key(Tree* tree, int key);
int tree_find_key_recursive(Node* node, int key);
int tree_find_key_iterative(Tree* tree, int key);
int tree_is_valid(Tree* tree);
Tree* tree_deep_copy(Tree* original);
void tree_delete(Tree* tree);

// Helper function prototypes
Node* create_node(int key);
int is_valid_bst(Node* node, int* min, int* max);
Node* deep_copy_node(Node* node);
void delete_node(Node* node);

// Implementation of tree_create
Tree* tree_create() {
    Tree* tree = (Tree*)malloc(sizeof(Tree));
    if (tree) {
        tree->root = NULL;
    }
    return tree;
}

// Implementation of tree_insert_key
Node* create_node(int key) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node) {
        new_node->key = key;
        new_node->smaller_keys = NULL;
        new_node->larger_keys = NULL;
    }
    return new_node;
}

void tree_insert_key(Tree* tree, int key) {
    if (!tree->root) {
        tree->root = create_node(key);
        return;
    }

    Node* current = tree->root;
    while (1) {
        if (key < current->key) {
            if (!current->smaller_keys) {
                current->smaller_keys = create_node(key);
                break;
            }
            current = current->smaller_keys;
        } else if (key > current->key) {
            if (!current->larger_keys) {
                current->larger_keys = create_node(key);
                break;
            }
            current = current->larger_keys;
        } else {
            // Key already exists, do nothing
            break;
        }
    }
}

// Implementation of tree_find_key_recursive
int tree_find_key_recursive(Node* node, int key) {
    if (!node) return 0;
    if (node->key == key) return 1;
    if (key < node->key) return tree_find_key_recursive(node->smaller_keys, key);
    return tree_find_key_recursive(node->larger_keys, key);
}

// Implementation of tree_find_key_iterative
int tree_find_key_iterative(Tree* tree, int key) {
    Node* current = tree->root;
    while (current) {
        if (current->key == key) return 1;
        if (key < current->key) current = current->smaller_keys;
        else current = current->larger_keys;
    }
    return 0;
}

// Implementation of tree_is_valid
int is_valid_bst(Node* node, int* min, int* max) {
    if (!node) return 1;
    
    if ((min && node->key <= *min) || (max && node->key >= *max)) return 0;
    
    return is_valid_bst(node->smaller_keys, min, &node->key) &&
           is_valid_bst(node->larger_keys, &node->key, max);
}

int tree_is_valid(Tree* tree) {
    return is_valid_bst(tree->root, NULL, NULL);
}

// Implementation of tree_deep_copy
Node* deep_copy_node(Node* node) {
    if (!node) return NULL;
    
    Node* new_node = create_node(node->key);
    new_node->smaller_keys = deep_copy_node(node->smaller_keys);
    new_node->larger_keys = deep_copy_node(node->larger_keys);
    
    return new_node;
}

Tree* tree_deep_copy(Tree* original) {
    Tree* copy = tree_create();
    copy->root = deep_copy_node(original->root);
    return copy;
}

// Implementation of tree_delete
void delete_node(Node* node) {
    if (!node) return;
    delete_node(node->smaller_keys);
    delete_node(node->larger_keys);
    free(node);
}

void tree_delete(Tree* tree) {
    if (tree) {
        delete_node(tree->root);
        free(tree);
    }
}

// Main function for testing (optional)
int main() {
    Tree* tree = tree_create();
    int choice, key;

    while (1) {
        printf("\nBinary Search Tree Operations:\n");
        printf("1. Insert a key\n");
        printf("2. Find a key (recursive)\n");
        printf("3. Find a key (iterative)\n");
        printf("4. Check if tree is valid\n");
        printf("5. Create a deep copy\n");
        printf("6. Delete the tree and exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter key to insert: ");
                scanf("%d", &key);
                tree_insert_key(tree, key);
                printf("Key inserted.\n");
                break;
            case 2:
                printf("Enter key to find: ");
                scanf("%d", &key);
                if (tree_find_key_recursive(tree->root, key))
                    printf("Key found (recursive).\n");
                else
                    printf("Key not found (recursive).\n");
                break;
            case 3:
                printf("Enter key to find: ");
                scanf("%d", &key);
                if (tree_find_key_iterative(tree, key))
                    printf("Key found (iterative).\n");
                else
                    printf("Key not found (iterative).\n");
                break;
            case 4:
                if (tree_is_valid(tree))
                    printf("The tree is valid.\n");
                else
                    printf("The tree is not valid.\n");
                break;
            case 5:
                {
                    Tree* copy = tree_deep_copy(tree);
                    printf("Deep copy created.\n");
                    if (tree_is_valid(copy))
                        printf("The copy is valid.\n");
                    else
                        printf("The copy is not valid.\n");
                    tree_delete(copy);
                    printf("Deep copy deleted.\n");
                }
                break;
            case 6:
                tree_delete(tree);
                printf("Tree deleted. Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}