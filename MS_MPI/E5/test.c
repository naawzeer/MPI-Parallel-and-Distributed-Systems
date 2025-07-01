#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void prefix_sequential(int* in, int in_length, int** outp) {
    int* out = (int*)malloc(in_length * sizeof(int));
    out[0] = 0;
    for (int i = 1; i < in_length; i++) {
        out[i] = out[i - 1] + in[i - 1];
    }
    *outp = out;
}

void prefix_mpi(int* in, int in_length, int block_size, int** outp) {
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int block_count = in_length / block_size;
    int* block_sum = (int*)malloc(num_procs * sizeof(int));
    int* block_prefix = (int*)malloc(num_procs * sizeof(int));

    // Step 1: Local sum of each block
    int local_sum = 0;
    for (int i = 0; i < block_size; i++) {
        local_sum += in[i];
    }
    MPI_Allgather(&local_sum, 1, MPI_INT, block_sum, 1, MPI_INT, MPI_COMM_WORLD);

    // Step 2: Parallel prefix sum across block sums
    block_prefix[0] = 0;
    for (int i = 1; i < num_procs; i++) {
        block_prefix[i] = block_prefix[i - 1] + block_sum[i - 1];
    }

    // Step 3: Distribute the prefix sum results back to the respective blocks
    int* out = (int*)malloc(block_size * sizeof(int));
    out[0] = block_prefix[rank];
    for (int i = 1; i < block_size; i++) {
        out[i] = out[i - 1] + in[i - 1];
    }

    *outp = out;
    free(block_sum);
    free(block_prefix);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int in_length = 1000;  // Example length of the array
    int block_size = in_length / num_procs;
    int* in = (int*)malloc(in_length * sizeof(int));
    int* sequential_out = NULL;
    int* parallel_out = NULL;

    // Generate random input array
    srand(time(NULL));
    for (int i = 0; i < in_length; i++) {
        in[i] = rand() % 11;  // Random numbers from 0 to 10
    }

    // Sequential prefix sum for verification
    if (rank == 0) {
        prefix_sequential(in, in_length, &sequential_out);
    }

    // Scatter the input array to all processes
    int* local_in = (int*)malloc(block_size * sizeof(int));
    MPI_Scatter(in, block_size, MPI_INT, local_in, block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Parallel prefix sum
    prefix_mpi(local_in, block_size, block_size, &parallel_out);

    // Gather the results back to the master process
    int* gathered_parallel_out = (int*)malloc(in_length * sizeof(int));
    MPI_Gather(parallel_out, block_size, MPI_INT, gathered_parallel_out, block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Verification on the master process
    if (rank == 0) {
        int error = 0;
        for (int i = 0; i < in_length; i++) {
            if (sequential_out[i] != gathered_parallel_out[i]) {
                error = 1;
                break;
            }
        }
        if (error) {
            printf("Error: Sequential and parallel results do not match.\n");
        } else {
            printf("Success: Sequential and parallel results match.\n");
        }
    }

    // Clean up
    if (rank == 0) {
        free(sequential_out);
    }
    free(local_in);
    free(parallel_out);
    if (rank == 0) {
        free(gathered_parallel_out);
    }

    MPI_Finalize();
    return 0;
}
