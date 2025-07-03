#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>  
#include <stdbool.h>

void reduce_tree(
    int* send_data,
    int* recv_data,
    int count,
    MPI_Comm communicator)
{
    int my_rank;
    int com_size;
    MPI_Comm_rank(communicator, &my_rank);
    MPI_Comm_size(communicator, &com_size);

    int* my_partial_sum;
    if (my_rank == 0)
        my_partial_sum = recv_data;
    else
        my_partial_sum = (int*)malloc(count * sizeof(int));

    memcpy(my_partial_sum, send_data, count * sizeof(int));
    
    int my_parent_rank = (my_rank - 1) / 2;
    int left_child_rank = 2 * my_rank + 1;
    int right_child_rank = 2 * my_rank + 2;

    bool left_child_exists = left_child_rank < com_size;
    bool right_child_exists = right_child_rank < com_size;


    int* left_child_recv_buffer = NULL;
    MPI_Request left_child_request = MPI_REQUEST_NULL;
    if (left_child_exists)
    {
        left_child_recv_buffer = (int*)malloc(count* sizeof(int));
        MPI_Irecv(left_child_recv_buffer, count, MPI_INT, 
            left_child_rank, 0, communicator, &left_child_request);
    }

    int* right_child_recv_buffer = NULL;
    MPI_Request right_child_request = MPI_REQUEST_NULL;
    if (right_child_exists)
    {
        right_child_recv_buffer = (int*)malloc(count * sizeof(int));
        MPI_Irecv(right_child_recv_buffer, count, MPI_INT,
            right_child_rank, 0, communicator, &right_child_request);
    }

    if (left_child_exists)
    {
        MPI_Wait(&left_child_request, MPI_STATUS_IGNORE);
        for (int i = 0; i < count; i++)
            my_partial_sum[i] += left_child_recv_buffer[i];
        free(left_child_recv_buffer);
    }

    if (right_child_exists)
    {
        MPI_Wait(&right_child_request, MPI_STATUS_IGNORE);
        for (int i = 0; i < count; i++)
            my_partial_sum[i] += right_child_recv_buffer[i];
        free(right_child_recv_buffer);
    }

    if (my_rank != 0)
    {
        MPI_Send(my_partial_sum, count, MPI_INT, my_parent_rank, 0, communicator);
        free(my_partial_sum);
    }
}

void reduce_sequential(
    int* send_data,
    int* recv_data,
    int count,
    MPI_Comm communicator)
{
    int my_rank;
    int com_size;
    MPI_Comm_rank(communicator, &my_rank);
    MPI_Comm_size(communicator, &com_size);

    int* gather_buffer = NULL;
    if (my_rank == 0)
    {
        gather_buffer = (int*) calloc(count * com_size, sizeof(int));
    }

    MPI_Gather(send_data, count, MPI_INT, gather_buffer, count, MPI_INT, 0, communicator);

    if (my_rank == 0)
    {
        memset(recv_data, 0, count * sizeof(int));
        for (int p = 0; p < com_size; p++)
            for (int i = 0; i < count; i++)
                recv_data[i] += gather_buffer[count * p + i];
        free(gather_buffer);
    }
}



int main(int argc, char** args)
{
    MPI_Init(&argc, &args);
    int count = 1024;
    int max_value = 64;
    int* recv_array_tree = NULL;
    int* recv_array_sequential = NULL;

    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == 0)
    {
        recv_array_tree = (int*) malloc(count * sizeof(int));
        recv_array_sequential = (int*) malloc(count * sizeof(int));
    }

    int* send_array = (int*)malloc(count * sizeof(int));
    for (int i = 0; i < count; i++)
        send_array[i] = my_rank;

    reduce_tree(send_array, recv_array_tree, count, MPI_COMM_WORLD);
    reduce_sequential(send_array, recv_array_sequential, count, MPI_COMM_WORLD);

    if (my_rank == 0)
    {
        for (int i = 0; i < count; i++)
            if (recv_array_tree[i] != recv_array_sequential[i])
                printf("At index %i: reduce_tree is %i, reduce_sequential is %i\n",
                    i, recv_array_tree[i], recv_array_sequential[i]);

        free(recv_array_tree);
        free(recv_array_sequential);
    }
    free(send_array);
    MPI_Finalize();
    return 0;
}


// Subtask d)
//
// General assumption:
//    - For small arrays: Startup time dominates the runtime of a communication operation
//    - For large arrays: Bandwidth dominates the runtime of a communication operation
//      
// In the following: 
//      - We assume that our cluster has a star topology, i.e. a central network hub with a 
//        connection to each node of the cluster
//      - BW denotes the bandwidth of a connection from the central hub to a node
//        of the network
//                  
//
// Performance estimation for reduce_sequential: 
//      - Performance depends on the implementation of MPI_Gather
//      - If we assume that MPI_Gather establishes all connections concurrently in order to 
//        reduce the impact of the startup time, its total runtime can be estimated as: 
//   
//          t_total = count * sizeof(int) * (com_size - 1)  / BW + t_startup
// 
//      - This is because the data from all other nodes, which is in total 
//        count * sizeof(int) * (com_size-1) Bytes, has to transmitted across a single
//        connection from the central hub to the root node.
//      - While all nodes may potentially send their data to the central hub utilizing the
//        full bandwidth of their connection, all this data has pass through the connection
//        from the central hub to the root node
//      ->The connection from the central hub to the root node becomes a bottleneck
// 
// Performance estimation for the reduce_tree: 
//      - This implementation reduces the levels of the tree one after another
//      - Estimated runtime for each level of the tree:
//              t_level = 2 * count * sizeof(int) / BW + t_startup
//        - That is because:
//              - All children of a level may send data to their respective parent node in
//                parallel.
//              - While the two children of a parent node can potentially send their data to
//                the central hub utilizing the full bandwidth of both of their connections, 
//                their parent node can only recieve data from both of its children 
//                utilizing the bandwidth of its single connection.
//       
//      - Estimated runtime for the whole reduction operation:
//              t_total = (h - 1) * t_level 
//                      = 2 * count * sizeof(int) * (h - 1) / BW + (h - 1) * t_startup
//          with h being the height of the tree, which is log_2(com_size + 1)
//
//  - Comparision:
//      - For smaller arrays only consider impact of startup time on the runtime:
//          - reduce_sequential: t_startup 
//          - reduce_tree: (log_2(com_size + 1) - 1) * t_startup  
//          -> prefer reduce_sequential for smaller arrays
//      - For larger arrays only consider impact of bandwidth on the runtime:
//          - reduce_sequential: count * sizeof(int) * (com_size - 1) / BW
//          - reduce_tree:  2 * count * sizeof(int) * (log_2(com_size + 1) - 1) / BW
//          -> prefer reduce_tree for larger arrays