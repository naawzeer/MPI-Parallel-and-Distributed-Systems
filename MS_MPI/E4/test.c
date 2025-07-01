#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NODES 100

// Tree structure as an adjacency list
typedef struct {
    int children[MAX_NODES];
    int child_count;
} TreeNode;

// DFS to compute the depth of the tree
int compute_depth(TreeNode *tree, int node, int depth) {
    if (tree[node].child_count == 0) {
        return depth;
    }
    int max_depth = depth;
    for (int i = 0; i < tree[node].child_count; i++) {
        int child = tree[node].children[i];
        int child_depth = compute_depth(tree, child, depth + 1);
        if (child_depth > max_depth) {
            max_depth = child_depth;
        }
    }
    return max_depth;
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    TreeNode tree[MAX_NODES];  
    int num_nodes = 7; // Example: A tree with 7 nodes

    // Root process initializes the tree
    if (rank == 0) {
        for (int i = 0; i < MAX_NODES; i++) {
            tree[i].child_count = 0;
        }

        // Example tree (Adjacency list representation)
        tree[0].children[0] = 1;
        tree[0].children[1] = 2;
        tree[0].child_count = 2;

        tree[1].children[0] = 3;
        tree[1].children[1] = 4;
        tree[1].child_count = 2;

        tree[2].children[0] = 5;
        tree[2].children[1] = 6;
        tree[2].child_count = 2;


        tree[3].children[0] = 9;
        tree[3].child_count = 1;
        
        tree[4].child_count = 0;
        tree[5].child_count = 0;
        tree[6].child_count = 0;


    }

    // Scatter tree data (for simplicity, we send the entire tree to all)
    MPI_Bcast(tree, MAX_NODES * sizeof(TreeNode), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Each process computes depth for a subset of nodes
    int local_max_depth = 0;
    for (int i = rank; i < num_nodes; i += size) {
        int depth = compute_depth(tree, i, 1);
        if (depth > local_max_depth) {
            local_max_depth = depth;
        }
    }

    // Find global maximum depth
    int global_max_depth;
    MPI_Reduce(&local_max_depth, &global_max_depth, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    // Root prints the maximum depth
    if (rank == 0) {
        printf("Maximum Depth of the Tree: %d\n", global_max_depth);
    }

    MPI_Finalize();
    return 0;
}
