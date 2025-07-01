#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void swap(double** a, double** b) {
    double* temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char** args) {
    MPI_Init(&argc, &args);
    
    int num_procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    double delta_t = 0.02;
    int grid_size = 4*8*8;
    int block_size = grid_size / num_procs;
    int num_time_steps = 10;
    double conductivity = 0.1;
    
    double* T_k = (double*) malloc(sizeof(double) * grid_size);
    double* T_kn = (double*) malloc(sizeof(double) * grid_size);
    
    int start = block_size * rank;
    int end = block_size * (rank + 1);
    
    for (int i = start; i < end; i++) {
        T_k[i] = i;
    }

    for (int k = 0; k < num_time_steps; k++) {
        if (num_procs > 1) {
            if (rank > 0) {
                MPI_Send(&T_k[start], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
                MPI_Recv(&T_k[start - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if (rank < num_procs - 1) {
                MPI_Send(&T_k[end - 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
                MPI_Recv(&T_k[end], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        
        for (int i = start; i < end; i++) {
            int i_left = (i != start) ? i - 1 : start;
            int i_right = (i != end - 1) ? i + 1 : end - 1;
            double dTdt_i = conductivity * (-2 * T_k[i] + T_k[i_left] + T_k[i_right]);
            T_kn[i] = T_k[i] + delta_t * dTdt_i;
        }
        swap(&T_k, &T_kn);
    }

    double T_average = 0;
    for (int i = start; i < end; i++) {
        T_average += T_k[i];
    }

    double global_T_average = 0;
    MPI_Reduce(&T_average, &global_T_average, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    global_T_average /= grid_size;

    if (rank == 0) {
        printf("T_average: %f\n", global_T_average);
    }

    free(T_k);
    free(T_kn);
    
    MPI_Finalize();
    return 0;
}
