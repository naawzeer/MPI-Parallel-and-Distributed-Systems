#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

typedef struct NODE Node;

struct NODE {
	int key;
	Node* smaller_keys;
	Node* larger_keys;
};

typedef struct {
	Node* root;
}Tree;

void tree_create(Tree** tree)
{
	*tree = (Tree*)malloc(sizeof(Tree));
	(*tree)->root = NULL;
}

// Alternative: No dynamic allocation, only initialization
// In this case, the caller is responosible for allocating
// the tree structure
void tree_create_only_init(Tree* tree)
{
	tree->root = NULL;
}


void node_create(Node** node, int key)
{
	*node = (Node*)malloc(sizeof(Node));
	(*node)->larger_keys = NULL;
	(*node)->smaller_keys = NULL;
	(*node)->key = key;
}



void tree_insert_key(Tree* tree, int key)
{
	if(tree->root == NULL)
	{
		node_create(&tree->root, key);	
		return;
	}
	
	Node* it_n = tree->root;
	while (true)
	{
		if (it_n->key < key)
		{
			if(it_n->left_child == NULL)
			{
				node_create(&it_n->left_child, key);
				return;
			}
			else
				it_n = it_n->left_child;

		}		
		else if (it_n->key > key)
		{
			if(it_n->right_child == NULL)
			{
				node_create(&it_n->right_child, key);
				return;
			}
			else
				it_n = it_n->right_child;

		}
		else 
			return;
	}
}


// Alternative: Using a pointer on the pointer, which
// points on the next node to traverse or the new node
// to create. By this, the conditionals can be simplified.
void tree_insert_key(Tree* tree, int key)
{
	Node** it_n = &tree->root;
	while (true)
	{
		if (*it_n == NULL)
		{
			node_create(it_n, key);
			return;
		}

		if ((*it_n)->key == key)
			return;

		it_n = key < (*it_n)->key ?
			   &((*it_n)->smaller_keys) : 
			   &((*it_n)->larger_keys);
	}
}

Node* subtree_find_key_recursive(Node* subtree, int key)
{
	if (subtree == NULL)
		return NULL;

	if (subtree->key == key)
		return subtree;

	return subtree_find_key_recursive(key < subtree->key ? 
		subtree->smaller_keys : subtree->larger_keys, key);
}

Node* tree_find_key_recursive(Tree* tree, int key)
{
	return subtree_find_key_recursive(tree->root, key);
}

Node* tree_find_key_iterative(Tree* tree, int key)
{
	Node* it_n = tree->root;
	while (true)
	{
		if (it_n == NULL)
			return NULL;

		if (it_n->key == key)
			return it_n;

		it_n = key < it_n->key ? it_n->smaller_keys : it_n->larger_keys;
	}
}

bool subtree_is_valid(Node* subtree, int min_bound, int max_bound)
{
	if (subtree == NULL)
		return true;

	int key = subtree->key;
	if (key < min_bound || key > max_bound)
		return false;

	return
		subtree_is_valid(subtree->smaller_keys, min_bound, key - 1) &&
		subtree_is_valid(subtree->larger_keys, key + 1, max_bound);
}

bool tree_is_valid(Tree* tree)
{
	return subtree_is_valid(tree->root, INT_MIN, INT_MAX);
}

void subtree_deep_copy(Node** new_subtree, Node* old_subtree)
{
	if (old_subtree == NULL)
		return;

	node_create(new_subtree, old_subtree->key);
	subtree_deep_copy(&((*new_subtree)->smaller_keys), old_subtree->smaller_keys);
	subtree_deep_copy(&((*new_subtree)->larger_keys), old_subtree->larger_keys);
}

void tree_deep_copy(Tree** new_tree, Tree* old_tree)
{
	tree_create(new_tree);
	subtree_deep_copy(&(*new_tree)->root, old_tree->root);
}

void subtree_delete(Node* subtree)
{
	if (subtree == NULL)
		return;

	subtree_delete(subtree->smaller_keys);
	subtree_delete(subtree->larger_keys);

	free(subtree);
}

void tree_delete(Tree* tree)
{
	subtree_delete(tree->root);
}



int main(int argc, char** args)
{
	int key_count = 1000;
	int max_key = 3000;
	int min_key = -3000;

	int* keys = (int*)malloc(sizeof(int) * key_count);
	int* non_keys = (int*)malloc(sizeof(int) * key_count);
	srand((unsigned int)time(NULL));

	Tree* t_1; tree_create(&t_1);
	Tree* t_inv; tree_create(&t_inv);


	for (int i = 0; i < key_count; i++)
	{
		keys[i] = rand() % (max_key - min_key) - min_key;

		bool key_added_before = false;
		for (int j = 0; j < i; j++)
			if (keys[j] == keys[i])
				key_added_before = true;

		if (key_added_before)
			i--;
	}

	for (int i = 0; i < key_count; i++)
	{
		non_keys[i] = rand() % (max_key - min_key) - min_key;

		bool key_added = false;
		for (int j = 0; j < key_count; j++)
			if (non_keys[i] == keys[j])
				key_added = true;

		if (key_added)
			i--;
	}

	for (int i = 0; i < key_count; i++)
	{
		tree_insert_key(t_1, keys[i]);
		tree_insert_key(t_2, keys[i]);
		tree_insert_key(t_inv, keys[i]);
	}

	for (int i = 0; i < key_count; i++)
		if (tree_find_key_recursive(t_1, keys[i]) == NULL)
			printf("ERROR! Insterted key not found recursively!\n");

	for (int i = 0; i < key_count; i++)
		if (tree_find_key_recursive(t_1, non_keys[i]) != NULL)
			printf("ERROR! Uninserted key found recursively!\n");

	for (int i = 0; i < key_count; i++)
		if (tree_find_key_iterative(t_1, keys[i]) == NULL)
			printf("ERROR! Inserted key not found iteratively!\n");

	for (int i = 0; i < key_count; i++)
		if (tree_find_key_iterative(t_1, non_keys[i]) != NULL)
			printf("ERROR! Uninserted key found recursively!\n");


	if (tree_is_valid(t_1) != true)
		printf("ERROR! VALID TREE CONSIDERED INVALID!\n");

	for (int test_id = 0; test_id < 1000; test_id++)
	{
		int path_length = rand() % 10;
		Node* node_inv = t_inv->root;
		for (int i = 0; i < path_length; i++)
		{
			if (node_inv->larger_keys == NULL || node_inv->smaller_keys == NULL)
				break;

			Node* new_node;

			if (rand() % 2)
				new_node = node_inv->larger_keys;
			else
				new_node = node_inv->smaller_keys;

			if (new_node != NULL)
				node_inv = new_node;
		}
		 
		Node* node_it = node_inv->smaller_keys;
		if (node_it != NULL)
		{
			while (node_it->larger_keys != NULL)
				node_it = node_it->larger_keys;

			int old_key = node_it->key;
			node_it->key = node_inv->key;
			if (tree_is_valid(t_inv) == true)
				printf("ERROR! INVALID TREE CONSIDERED VALID!\n");
			node_inv->key = old_key;
		}

		node_it = node_inv->larger_keys;
		if (node_it != NULL)
		{
			while (node_it->smaller_keys != NULL)
				node_it = node_it->smaller_keys;

			int old_key = node_it->key;
			node_it->key = node_inv->key;
			if (tree_is_valid(t_inv) == true)
				printf("ERROR! Invalid tree considered valid!\n");
			node_it->key = old_key;
		}
	}


	Tree* t_copy;
	tree_deep_copy(&t_copy, t_1);
	for (int i = 0; i < key_count; i++)
		if (tree_find_key_recursive(t_copy, keys[i]) == NULL)
			printf("ERROR! Key lost by copying tree!\n");

	if (tree_is_valid(t_copy) != true)
		printf("ERROR! DEPP COPY CONSIDERED INVALID!\n");
		
	free(keys);
	free(non_keys);
	
	tree_delete(t_1);
	tree_delete(t_inv);	
	tree_delete(t_copy);	
		
	return 0;
}
