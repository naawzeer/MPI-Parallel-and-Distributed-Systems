#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct GRAPH Graph;
typedef struct NODE Node;

struct NODE {
    Graph* graph;
    char* label;
    int adjacent_nodes_count;
    Node** adjacent_nodes;
};

struct GRAPH {
    int node_count;
    Node** nodes;
};

void graph_create(Graph** g) {
    *g = (Graph*)malloc(sizeof(Graph));
    if (*g == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    (*g)->node_count = 0;
    (*g)->nodes = NULL;
}

int graph_get_node_id(Node* n) {
    for (int n_id = 0; n_id < n->graph->node_count; n_id++)
        if (n->graph->nodes[n_id] == n)
            return n_id;
    printf("ERROR NODE NOT FOUND\n");
    return -1;
}

Node* graph_insert_node(Graph* g, char* label) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (n == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    n->adjacent_nodes_count = 0;
    n->adjacent_nodes = NULL;
    n->label = (char*)malloc((strlen(label) + 1) * sizeof(char));
    if (n->label == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(n);
        exit(EXIT_FAILURE);
    }
    strcpy(n->label, label);
    n->graph = g;
    g->node_count++;
    g->nodes = (Node**)realloc(g->nodes, g->node_count * sizeof(Node*));
    if (g->nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    g->nodes[g->node_count - 1] = n;
    return n;
}

void graph_insert_edge(Node* n_1, Node* n_2) {
    n_1->adjacent_nodes_count++;
    n_1->adjacent_nodes = (Node**)realloc(n_1->adjacent_nodes, n_1->adjacent_nodes_count * sizeof(Node*));
    if (n_1->adjacent_nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    n_1->adjacent_nodes[n_1->adjacent_nodes_count - 1] = n_2;

    n_2->adjacent_nodes_count++;
    n_2->adjacent_nodes = (Node**)realloc(n_2->adjacent_nodes, n_2->adjacent_nodes_count * sizeof(Node*));
    if (n_2->adjacent_nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    n_2->adjacent_nodes[n_2->adjacent_nodes_count - 1] = n_1;
}

void graph_delete(Graph* g) {
    for (int n_id = 0; n_id < g->node_count; n_id++) {
        free(g->nodes[n_id]->adjacent_nodes);
        free(g->nodes[n_id]->label);
        free(g->nodes[n_id]);
    }
    free(g->nodes);
    free(g);
}

typedef struct {
    int node_count;
    Node** nodes;
} Path;

void path_print(Path* p) {
    for (int n_id = 0; n_id < p->node_count; n_id++) {
        printf("%s", p->nodes[n_id]->label);
        if (n_id != p->node_count - 1)
            printf("->");
    }
    printf("\n");
}

void path_delete(Path* p) {
    free(p->nodes);
    free(p);
}

void graph_find_shortest_path(Node* from, Node* to, Path** p) {
    *p = (Path*)malloc(sizeof(Path));
    if (*p == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    if (from == to) {
        (*p)->node_count = 1;
        (*p)->nodes = (Node**)malloc(sizeof(Node*));
        if ((*p)->nodes == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(*p);
            exit(EXIT_FAILURE);
        }
        (*p)->nodes[0] = from;
        return;
    }

    Graph* g = from->graph;

    unsigned int* nodes_current_cost = (unsigned int*)malloc(g->node_count * sizeof(unsigned int));
    if (nodes_current_cost == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int n_id = 0; n_id < g->node_count; n_id++)
        nodes_current_cost[n_id] = UINT_MAX;
    nodes_current_cost[graph_get_node_id(from)] = 0;

    int* nodes_best_reachable_from = (int*)malloc(g->node_count * sizeof(int));
    if (nodes_best_reachable_from == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(nodes_current_cost);
        exit(EXIT_FAILURE);
    }
    for (int n_id = 0; n_id < g->node_count; n_id++)
        nodes_best_reachable_from[n_id] = -1;

    bool* nodes_added = (bool*)malloc(g->node_count * sizeof(bool));
    if (nodes_added == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(nodes_current_cost);
        free(nodes_best_reachable_from);
        exit(EXIT_FAILURE);
    }
    for (int n_id = 0; n_id < g->node_count; n_id++)
        nodes_added[n_id] = false;
    nodes_added[graph_get_node_id(from)] = true;

    for (int e_id = 0; e_id < from->adjacent_nodes_count; e_id++) {
        Node* n = from->adjacent_nodes[e_id];
        nodes_best_reachable_from[graph_get_node_id(n)] = graph_get_node_id(from);
        nodes_current_cost[graph_get_node_id(n)] = 1;
    }

    bool node_found = false;
    while (true) {
        unsigned int next_min_cost = UINT_MAX;
        int next_node_id = -1;
        for (int n_id = 0; n_id < g->node_count; n_id++)
            if (nodes_added[n_id] == false &&
                nodes_current_cost[n_id] < next_min_cost) {
                next_min_cost = nodes_current_cost[n_id];
                next_node_id = n_id;
            }
        if (next_min_cost == UINT_MAX)
            break;
        nodes_added[next_node_id] = true;
        Node* next_node = g->nodes[next_node_id];
        if (next_node == to) {
            node_found = true;
            break;
        }
        for (int e_id = 0; e_id < next_node->adjacent_nodes_count; e_id++) {
            int updated_node_id = graph_get_node_id(next_node->adjacent_nodes[e_id]);
            unsigned int cost_to_reach = 1 + nodes_current_cost[next_node_id];
            if (nodes_current_cost[updated_node_id] > cost_to_reach) {
                nodes_current_cost[updated_node_id] = cost_to_reach;
                nodes_best_reachable_from[updated_node_id] = next_node_id;
            }
        }
    }

    if (!node_found) {
        (*p)->node_count = 0;
        (*p)->nodes = NULL;
    } else {
        int to_node_id = graph_get_node_id(to);
        int from_node_id = graph_get_node_id(from);

        (*p)->nodes = NULL;
        (*p)->node_count = 1;
        int backtrack_node_id = to_node_id;
        while (backtrack_node_id != from_node_id) {
            (*p)->node_count++;
            backtrack_node_id = nodes_best_reachable_from[backtrack_node_id];
        }
        (*p)->nodes = (Node**)malloc((*p)->node_count * sizeof(Node*));
        if ((*p)->nodes == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(nodes_current_cost);
            free(nodes_best_reachable_from);
            free(nodes_added);
            exit(EXIT_FAILURE);
        }
        (*p)->nodes[(*p)->node_count - 1] = to;
        backtrack_node_id = to_node_id;
        for (int n_id = 1; n_id < (*p)->node_count; n_id++) {
            backtrack_node_id = nodes_best_reachable_from[backtrack_node_id];
            Node* current_backtrack_node = g->nodes[backtrack_node_id];
            (*p)->nodes[(*p)->node_count - 1 - n_id] = current_backtrack_node;
        }
    }
    free(nodes_current_cost);
    free(nodes_best_reachable_from);
    free(nodes_added);
}

int graph_compute_degree(Graph* g) {
    if (g->node_count == 0) {
        return 0;
    }
    int max_degree = 0;
    for (int i = 0; i < g->node_count; i++) {
        if (g->nodes[i]->adjacent_nodes_count > max_degree) {
            max_degree = g->nodes[i]->adjacent_nodes_count;
        }
    }
    return max_degree;
}

int graph_compute_diameter(Graph* g) {
    if (g->node_count == 0) {
        return 0;
    }
    int diameter = 0;
    for (int i = 0; i < g->node_count; i++) {
        for (int j = i + 1; j < g->node_count; j++) {
            Path* p;
            graph_find_shortest_path(g->nodes[i], g->nodes[j], &p);
            if (p->node_count > diameter) {
                diameter = p->node_count - 1; // Path length is number of edges
            }
            path_delete(p);
        }
    }
    return diameter;
}

int graph_count_edges(Graph* g) {
    int edge_count = 0;
    for (int i = 0; i < g->node_count; i++) {
        edge_count += g->nodes[i]->adjacent_nodes_count;
    }
    return edge_count / 2; // Each edge is counted twice
}

void graph_create_ring(Graph** g, int num_nodes) {
    graph_create(g);
    if (num_nodes <= 0) {
        printf("Number of nodes must be greater than 0\n");
        return;
    }

    Node** nodes = (Node**)malloc(num_nodes * sizeof(Node*));
    if (nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_nodes; i++) {
        char label[10];
        sprintf(label, "N%d", i);
        nodes[i] = graph_insert_node(*g, label);
    }

    for (int i = 0; i < num_nodes; i++) {
        graph_insert_edge(nodes[i], nodes[(i + 1) % num_nodes]);
    }

    free(nodes);
}

void graph_create_3d_torus(Graph** g, int height, int width, int depth) {
    graph_create(g);
    if (height <= 0 || width <= 0 || depth <= 0) {
        printf("Dimensions must be greater than 0\n");
        return;
    }

    int total_nodes = height * width * depth;
    Node*** nodes = (Node***)malloc(height * sizeof(Node**));
    if (nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int h = 0; h < height; h++) {
        nodes[h] = (Node**)malloc(width * sizeof(Node*));
        if (nodes[h] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        for (int w = 0; w < width; w++) {
            nodes[h][w] = (Node*)malloc(depth * sizeof(Node*));
            if (nodes[h][w] == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            for (int d = 0; d < depth; d++) {
                char label[20];
                sprintf(label, "N%d_%d_%d", h, w, d);
                nodes[h][w][d] = graph_insert_node(*g, label);
            }
        }
    }

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            for (int d = 0; d < depth; d++) {
                graph_insert_edge(nodes[h][w][d], nodes[(h + 1) % height][w][d]);
                graph_insert_edge(nodes[h][w][d], nodes[h][(w + 1) % width][d]);
                graph_insert_edge(nodes[h][w][d], nodes[h][w][(d + 1) % depth]);
            }
        }
    }

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            free(nodes[h][w]);
        }
        free(nodes[h]);
    }
    free(nodes);
}

void graph_create_complete_graph(Graph** g, int num_nodes) {
    graph_create(g);
    if (num_nodes <= 0) {
        printf("Number of nodes must be greater than 0\n");
        return;
    }

    Node** nodes = (Node**)malloc(num_nodes * sizeof(Node*));
    if (nodes == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_nodes; i++) {
        char label[10];
        sprintf(label, "N%d", i);
        nodes[i] = graph_insert_node(*g, label);
    }

    for (int i = 0; i < num_nodes; i++) {
        for (int j = i + 1; j < num_nodes; j++) {
            graph_insert_edge(nodes[i], nodes[j]);
        }
    }

    free(nodes);
}

void test_graph(Graph* g, int expected_nodes, int expected_edges, int expected_degree, int expected_diameter) {
    int actual_nodes = g->node_count;
    int actual_edges = graph_count_edges(g);
    int actual_degree = graph_compute_degree(g);
    int actual_diameter = graph_compute_diameter(g);

    printf("Nodes: Expected %d, Got %d\n", expected_nodes, actual_nodes);
    printf("Edges: Expected %d, Got %d\n", expected_edges, actual_edges);
    printf("Degree: Expected %d, Got %d\n", expected_degree, actual_degree);
    printf("Diameter: Expected %d, Got %d\n", expected_diameter, actual_diameter);
    printf("\n");
}

int main(int argc, char** args) {
    // Test Ring Graph
    Graph* ring_graph;
    int ring_nodes = 5;
    graph_create_ring(&ring_graph, ring_nodes);
    printf("Testing Ring Graph:\n");
    test_graph(ring_graph, ring_nodes, ring_nodes, 2, ring_nodes / 2);
    graph_delete(ring_graph);

    // Test 3D Torus Graph
    Graph* torus_graph;
    int torus_height = 2;
    int torus_width = 2;
    int torus_depth = 2;
    graph_create_3d_torus(&torus_graph, torus_height, torus_width, torus_depth);
    printf("Testing 3D Torus Graph:\n");
    int torus_nodes = torus_height * torus_width * torus_depth;
    int torus_edges = 3 * torus_nodes; // Each node has 3 edges in a 3D torus
    int torus_degree = 3; // Each node connects to 3 neighbors
    int torus_diameter = 3; // Diameter of a 2x2x2 torus
    test_graph(torus_graph, torus_nodes, torus_edges, torus_degree, torus_diameter);
    graph_delete(torus_graph);

    // Test Complete Graph
    Graph* complete_graph;
    int complete_nodes = 4;
    graph_create_complete_graph(&complete_graph, complete_nodes);
    printf("Testing Complete Graph:\n");
    int complete_edges = (complete_nodes * (complete_nodes - 1)) / 2; // Complete graph formula
    int complete_degree = complete_nodes - 1; // Each node connects to all other nodes
    int complete_diameter = 1; // Diameter of a complete graph is always 1
    test_graph(complete_graph, complete_nodes, complete_edges, complete_degree, complete_diameter);
    graph_delete(complete_graph);

    return 0;
}
