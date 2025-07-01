#include <mpi.h>
#define COUNT (32 * 1024 * 1024)
#define BLOCK_SIZE (16 * 1024) // Example block size

void Reduce_linear_array(float* sendbuf, float* recvbuf, MPI_Comm comm)
{
    int my_rank; MPI_Comm_rank(comm, &my_rank);
    int comm_size; MPI_Comm_size(comm, &comm_size);

    if (my_rank > 0 && my_rank < comm_size - 1)
    {
        int num_blocks = COUNT / BLOCK_SIZE;
        float* temp = malloc(BLOCK_SIZE * sizeof(float));
        MPI_Request req_recv, req_send;

        for (int b = 0; b < num_blocks; b++) {
            // Start receiving block b from right neighbor
            MPI_Irecv(temp, BLOCK_SIZE, MPI_FLOAT, my_rank + 1, b, comm, &req_recv);

            // Compute and send previous block (if not the first block)
            if (b > 0) {
                MPI_Wait(&req_recv, MPI_STATUS_IGNORE);
                for (int i = 0; i < BLOCK_SIZE; i++)
                    temp[i] += sendbuf[(b-1)*BLOCK_SIZE + i];
                MPI_Isend(temp, BLOCK_SIZE, MPI_FLOAT, my_rank - 1, b-1, comm, &req_send);
                MPI_Wait(&req_send, MPI_STATUS_IGNORE);
            }

            // For the last block, handle after loop
        }

        // Handle the last block
        MPI_Wait(&req_recv, MPI_STATUS_IGNORE);
        for (int i = 0; i < BLOCK_SIZE; i++)
            temp[i] += sendbuf[(num_blocks-1)*BLOCK_SIZE + i];
        MPI_Isend(temp, BLOCK_SIZE, MPI_FLOAT, my_rank - 1, num_blocks-1, comm, &req_send);
        MPI_Wait(&req_send, MPI_STATUS_IGNORE);

        free(temp);
    }
    else if (my_rank == 0 || my_rank == comm_size - 1)
        //... Code and optimization of boundary cases not relevant in this task
        ;
}