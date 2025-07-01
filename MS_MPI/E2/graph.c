#include <limits.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>

typedef struct GRAPH Graph;
typedef struct NODE Node;


struct NODE
{
    Graph* graph; // Graph containing the node
    char* label;  // A string containing a label for the node
    int adjacent_nodes_count;  // Amount of other nodes which are directly connected by edges to this node
    Node** adjacent_nodes;     // Other nodes which are directly connected by edges to this node
                               // Those nodes are stored via an array of pointers
};

struct GRAPH {
    int node_count; // Amount of nodes in the graph
    Node** nodes;   // The nodes contained by this graph
                    // Those nodes are stored via an array of pointers
};

/////////////////////////////////////////////////////////////////////////////////
// This function allocates an empty graph in the dynamic memory 
// and returns a pointer to this graph via its first argument
/////////////////////////////////////////////////////////////////////////////////

void graph_create(Graph** g)
{
    *g = (Graph*)malloc(sizeof(Graph));
    (*g)->node_count = 0;
    (*g)->nodes = NULL;
}


/////////////////////////////////////////////////////////////////////////////////
// This function is a helper function returning the id of a node
// in the graph of this node
/////////////////////////////////////////////////////////////////////////////////

int graph_get_node_id(Node* n)
{
    for (int n_id = 0; n_id < n->graph->node_count; n_id++)
        if (n->graph->nodes[n_id] == n)
            return n_id;
    printf("ERROR NODE NOT FOUND\n");
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////
// This function creates a node in dynamic memory with a given 
// string as label, inserts the created node in the graph passed
// as argument and returns a pointer on the created node 
/////////////////////////////////////////////////////////////////////////////////

Node* graph_insert_node(Graph* g, char* label)
{
    Node* n = (Node*)malloc(sizeof(Node));

    n->adjacent_nodes_count= 0;
    n->adjacent_nodes = NULL;

    n->label = (char*)malloc((strlen(label) + 1) * sizeof(char));
    strcpy(n->label, label);

    n->graph = g;
    g->node_count++;
    g->nodes = (Node**)realloc(g->nodes, g->node_count * sizeof(Node*));
    g->nodes[g->node_count - 1] = n;
    return n;
}


/////////////////////////////////////////////////////////////////////////////////
// Inserts an edge between the two nodes passed as arguments
/////////////////////////////////////////////////////////////////////////////////

void graph_insert_edge(Node* n_1, Node* n_2)
{
    n_1->adjacent_nodes_count++;
    n_1->adjacent_nodes = (Node**)realloc(n_1->adjacent_nodes, n_1->adjacent_nodes_count * sizeof(Node*));
    n_1->adjacent_nodes[n_1->adjacent_nodes_count - 1] = n_2;

    n_2->adjacent_nodes_count++;
    n_2->adjacent_nodes = (Node**)realloc(n_2->adjacent_nodes, n_2->adjacent_nodes_count * sizeof(Node*));
    n_2->adjacent_nodes[n_2->adjacent_nodes_count - 1] = n_1;
}

/////////////////////////////////////////////////////////////////////////////////
// Deletes a graph passed as argument
/////////////////////////////////////////////////////////////////////////////////

void graph_delete(Graph* g)
{
    for (int n_id = 0; n_id < g->node_count; n_id++)
    {
        if(g->nodes[n_id]->adjacent_nodes_count != 0){
            free(g->nodes[n_id]->adjacent_nodes);
            free(g->nodes[n_id]->label);
        }
            
        free(g->nodes[n_id]);
    }

    free(g->nodes);
    free(g);
}

typedef struct
{
    int node_count;  // Amount of nodes in the path                
    Node** nodes;    // The nodes contained in this graph
                     // Those nodes are stored via an array of pointers    
}Path;


/////////////////////////////////////////////////////////////////////////////////
// Prints the nodes in the path passed as argument
/////////////////////////////////////////////////////////////////////////////////

void path_print(Path* p)
{
    for (int n_id = 0; n_id < p->node_count; n_id++)
    {
        printf("%s", p->nodes[n_id]->label);
        if (n_id != p->node_count - 1)
            printf("->");
    }
    printf("\n");
}

/////////////////////////////////////////////////////////////////////////////////
// Deletes the path 
/////////////////////////////////////////////////////////////////////////////////

void path_delete(Path* p) 
{
    free(p->nodes);
    free(p);
}


/////////////////////////////////////////////////////////////////////////////////
// This function finds the shortest path between the two nodes passed
// as first and second arguments via Dijkstra, and returns a pointer to the 
// found path data structure via its third argument.
// The returned path does not contain any nodes if no path is found.
// If from == to then the returned path only contains from . 
// Since the function allocates the path in dynamic memory, the returned path 
// needs to be deleted via the path_delete function!
/////////////////////////////////////////////////////////////////////////////////

void graph_find_shortest_path(Node* from, Node* to, Path** p)
{
    // Allocates the path
    *p = (Path*)malloc(sizeof(Path));

    if (from == to) // From-Node == To-Node, returning path with only From-Node
    {
        (*p)->node_count =1;
        (*p)->nodes = (Node**)malloc(sizeof(Node*));        
        (*p)->nodes[0] = from;
        return;
    }

    Graph* g = from->graph;
    
    // Current cost to add the nodes to the shortest path
    unsigned int* nodes_current_cost = (unsigned int*)malloc(g->node_count * sizeof(unsigned int));
    for (int n_id = 0; n_id < g->node_count; n_id++)        
            nodes_current_cost[n_id] = UINT_MAX;
    nodes_current_cost[graph_get_node_id(from)] = 0;
    
    // Stores for each node, from which other node the cost for reaching this
    // node are currently lowest
    int* nodes_best_reachable_from = (int*)malloc(g->node_count * sizeof(int));
    for (int n_id = 0; n_id < g->node_count; n_id++)    
        nodes_best_reachable_from[n_id] = -1;

    
    // Nodes, to which the shortest path is known
    bool* nodes_added = (bool*)malloc(g->node_count * sizeof(bool));
    for (int n_id = 0; n_id < g->node_count; n_id++)    
            nodes_added[n_id] = false;
    nodes_added[graph_get_node_id(from)] = true;
    
    
    // Initializes the shortest path from From-Node to all adjacent nodes
    for (int e_id = 0; e_id < from->adjacent_nodes_count; e_id++)
    {
        Node* n = from->adjacent_nodes[e_id];
        nodes_best_reachable_from[graph_get_node_id(n)] = graph_get_node_id(from);
        nodes_current_cost[graph_get_node_id(n)] = 1;
    }
    
    
    bool node_found = false;
    while (true)
    {
        unsigned int next_min_cost = UINT_MAX;
        int next_node_id = -1;

        // Searches for the next node to add to the nodes to which
        // the shortest path is known
        for (int n_id = 0; n_id < g->node_count; n_id++)
            if (nodes_added[n_id] == false &&
                nodes_current_cost[n_id] < next_min_cost)
            {
                next_min_cost = nodes_current_cost[n_id];
                next_node_id = n_id;
            }

        if (next_min_cost == UINT_MAX)  // No more nodes reachable because graph 
            break;                      // is disconnected, To-Node has not been 
                                        // reached

        // Adding the newly found node to the nodes to which the shortest path is
        // known
        nodes_added[next_node_id] = true;
        Node* next_node = g->nodes[next_node_id];

        if (next_node == to) // Found the shortest path to To-node
        {
            node_found = true;
            break;
        }

        // Updates the cost to reach all other nodes if the path from the newly  
        // added node is cheaper than the path from an older node
        for (int e_id = 0; e_id < next_node->adjacent_nodes_count; e_id++)
        {
            int updated_node_id = graph_get_node_id(next_node->adjacent_nodes[e_id]);
            unsigned int cost_to_reach = 1 + nodes_current_cost[next_node_id];

            if (nodes_current_cost[updated_node_id] > cost_to_reach)
            {
                nodes_current_cost[updated_node_id] = cost_to_reach;
                nodes_best_reachable_from[updated_node_id] = next_node_id;
            }
        }
    }


    if (!node_found) // To-Node not reachable because graph is disconnected
    {
        (*p)->node_count = 0;
        (*p)->nodes = NULL;
    }
    else // Backtracking, building the path from To-Node to From-Node
    {

        int to_node_id = graph_get_node_id(to);
        int from_node_id = graph_get_node_id(from);


        (*p)->nodes = NULL;
        
        // Determining the amount of nodes on the path 
        (*p)->node_count = 1;   
        int backtrack_node_id = to_node_id;
        while (backtrack_node_id != from_node_id)
        {
            (*p)->node_count++;
            backtrack_node_id = nodes_best_reachable_from[backtrack_node_id];
        }

        // Allocating pointers for the nodes on the path
        (*p)->nodes = (Node**)malloc((*p)->node_count * sizeof(Node*));
        

        // Inserting the nodes on the path from To-Node to From-Node        
        (*p)->nodes[(*p)->node_count-1] = to;
        backtrack_node_id = to_node_id;
        for (int n_id = 1; n_id < (*p)->node_count; n_id++)
        {
            backtrack_node_id = nodes_best_reachable_from[backtrack_node_id];
            Node* current_backtrack_node = g->nodes[backtrack_node_id];
            (*p)->nodes[(*p)->node_count-1-n_id] = current_backtrack_node;
        }

    }

    //freeing all temporary arrays
    free(nodes_current_cost);
    free(nodes_best_reachable_from);
    free(nodes_added);
}

/////////////////////////////////////////////////////////////////////////////////
// main function demonstrating the use of the graph functions
/////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** args)
{
    Graph* g_1;
    graph_create(&g_1);
    Node* A = graph_insert_node(g_1, (char*)"A");
    Node* B = graph_insert_node(g_1, (char*)"B");
    Node* C = graph_insert_node(g_1, (char*)"C");
    Node* D = graph_insert_node(g_1, (char*)"D");
    Node* E = graph_insert_node(g_1, (char*)"E");
    Node* F = graph_insert_node(g_1, (char*)"F");
    Node* G = graph_insert_node(g_1, (char*)"G");
    graph_insert_edge(A, B);
    graph_insert_edge(A, C);
    graph_insert_edge(C, F);
    graph_insert_edge(C, E);
    graph_insert_edge(B, D);
    graph_insert_edge(E, G);
    graph_insert_edge(B, F);
    
    Path* p;
    graph_find_shortest_path(A, G, &p);
    path_print(p);
    path_delete(p);
    graph_delete(g_1);
    return 0;
}




