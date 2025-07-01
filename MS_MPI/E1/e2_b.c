#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#define NUM_KEYS 10
#define MAX_KEY 100

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
void display_tree(Node* root, int level) {
    if (root == NULL) return;
    
    display_tree(root->larger_keys, level + 1);
    
    for (int i = 0; i < level; i++) printf("    ");
    printf("%d\n", root->key);
    
    display_tree(root->smaller_keys, level + 1);
}

void show_tree(Tree* tree) {
    printf("Tree structure:\n");
    display_tree(tree->root, 0);
    printf("\n");
}

int main() {
    srand(time(NULL));  // Seed the random number generator

    // 1. Create a tree
    Tree* tree = tree_create();
    printf("1. Created an empty tree.\n\n");

    // 2. Insert random keys
    printf("2. Inserting random keys:\n");
    for (int i = 0; i < 10; i++) {
        int key = rand() % 100;
        printf("   Inserting %d\n", key);
        tree_insert_key(tree, key);
    }
    show_tree(tree);

    // 3. Test tree_find_key_recursive
    int key_to_find = tree->root->key;  // Choose root key to ensure it exists
    printf("3. Testing tree_find_key_recursive:\n");
    printf("   Searching for key %d: %s\n\n", key_to_find, 
           tree_find_key_recursive(tree->root, key_to_find) ? "Found" : "Not found");

    // 4. Test tree_find_key_iterative
    printf("4. Testing tree_find_key_iterative:\n");
    printf("   Searching for key %d: %s\n\n", key_to_find, 
           tree_find_key_iterative(tree, key_to_find) ? "Found" : "Not found");

    // 5. Test tree_is_valid
    printf("5. Testing tree_is_valid:\n");
    printf("   Is the tree valid? %s\n\n", tree_is_valid(tree) ? "Yes" : "No");

    // 6. Test tree_deep_copy
    printf("6. Testing tree_deep_copy:\n");
    Tree* copy = tree_deep_copy(tree);
    printf("   Created a deep copy of the tree.\n");
    printf("   Is the copy valid? %s\n", tree_is_valid(copy) ? "Yes" : "No");
    printf("   Original tree:\n");
    show_tree(tree);
    printf("   Copied tree:\n");
    show_tree(copy);

    // 7. Test tree_delete
    printf("7. Testing tree_delete:\n");
    printf("   Deleting the original tree...\n");
    tree_delete(tree);
    printf("   Deleting the copy of the tree...\n");
    tree_delete(copy);
    printf("   Trees deleted.\n");

    return 0;
}