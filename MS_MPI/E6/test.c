#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void reduce_grid(const void *sendbuf, void *recvbuf, int count, MPI_Comm comm) {
    int rank, size, coords[2], dims[2], periods[2], root[2];
    MPI_Comm_rank(comm, &rank);
    MPI_Cart_get(comm, 2, dims, periods, coords);
    
    int proc_count_x = dims[0];
    int proc_count_y = dims[1];
    
    // Root is at the center of the grid
    root[0] = proc_count_x / 2;
    root[1] = proc_count_y / 2;
    int root_rank;
    MPI_Cart_rank(comm, root, &root_rank);
    
    double local_value = *((double *)sendbuf);
    double reduced_value = local_value;
    MPI_Status status;
    
    // Determine neighboring ranks
    int left, right, up, down;
    MPI_Cart_shift(comm, 0, 1, &left, &right);
    MPI_Cart_shift(comm, 1, 1, &up, &down);
    
    // Reduction process: Send values towards the root process
    if (coords[0] < root[0]) {
        if (down != MPI_PROC_NULL) {
            MPI_Recv(&local_value, count, MPI_DOUBLE, down, 0, comm, &status);
            reduced_value += local_value;
        }
        if (up != MPI_PROC_NULL) {
            MPI_Send(&reduced_value, count, MPI_DOUBLE, up, 0, comm);
        }
    } else if (coords[0] > root[0]) {
        if (up != MPI_PROC_NULL) {
            MPI_Recv(&local_value, count, MPI_DOUBLE, up, 0, comm, &status);
            reduced_value += local_value;
        }
        if (down != MPI_PROC_NULL) {
            MPI_Send(&reduced_value, count, MPI_DOUBLE, down, 0, comm);
        }
    }
    
    if (coords[1] < root[1]) {
        if (right != MPI_PROC_NULL) {
            MPI_Recv(&local_value, count, MPI_DOUBLE, right, 0, comm, &status);
            reduced_value += local_value;
        }
        if (left != MPI_PROC_NULL) {
            MPI_Send(&reduced_value, count, MPI_DOUBLE, left, 0, comm);
        }
    } else if (coords[1] > root[1]) {
        if (left != MPI_PROC_NULL) {
            MPI_Recv(&local_value, count, MPI_DOUBLE, left, 0, comm, &status);
            reduced_value += local_value;
        }
        if (right != MPI_PROC_NULL) {
            MPI_Send(&reduced_value, count, MPI_DOUBLE, right, 0, comm);
        }
    }
    
    // The root process receives the final reduced value
    if (rank == root_rank) {
        *((double *)recvbuf) = reduced_value;
        printf("Final reduced sum at root (Process %d): %f\n", rank, reduced_value);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int dims[2] = {2, 2}; // Example 4x4 grid
    int periods[2] = {0, 0};
    MPI_Comm cart_comm;
    
    // Create 2D Cartesian topology
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);
    
    // Generate some local data (each process has a random double value)
    srand(rank + 1);
    double local_value = (double)(rand() % 10 + 1);
    double result;
    
    printf("Process %d has local value: %.2f\n", rank, local_value);
    
    // Perform reduction
    reduce_grid(&local_value, &result, 1, cart_comm);
    
    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}
