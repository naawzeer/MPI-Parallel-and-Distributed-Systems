#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int root_rank = 0;
    const int elements_per_process = 4;
    int array[world_size * elements_per_process];
    int receive_buffer[elements_per_process];

    // Root process initializes the array with values
    if (world_rank == root_rank) {
        printf("Root process initializing the array:\n");
        for (int i = 0; i < world_size * elements_per_process; i++) {
            array[i] = i + 1;
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    // Scatter the array from the root process to all processes
    MPI_Scatter(array, elements_per_process, MPI_INT, 
                receive_buffer, elements_per_process, MPI_INT, 
                root_rank, MPI_COMM_WORLD);

    // Each process prints its received chunk after scatter
    printf("[Process %d]: received elements:", world_rank);
    for (int i = 0; i < elements_per_process; i++) {
        printf(" %d", receive_buffer[i]);
    }
    printf("\n");

    // New part: Sending and Receiving data between processes
    int send_value = world_rank * 10;
    int recv_value;

    if (world_rank != 0) {
        // Non-root processes send a value to the root process
        MPI_Send(&send_value, 1, MPI_INT, root_rank, 0, MPI_COMM_WORLD);
    } else {
        // The root process receives values from all non-root processes
        for (int i = 1; i < world_size; i++) {
            MPI_Recv(&recv_value, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("[Process 0]: received %d from process %d\n", recv_value, i);
        }
    }

    MPI_Finalize();
    return 0;
}

