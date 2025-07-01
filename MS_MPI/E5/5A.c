#include <mpi.h>
#include <stdio.h>

int reduce_sequential(int **num_array, int array_size) {
    int local_sum;

    return local_sum;
}

int main(int argc, char **args) {
    MPI_Init(&argc, &args);
    int my_rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    int **num_array = (int*)malloc(comm_size*sizeof(int));
    int **sum_array = (int*)malloc(comm_size*sizeof(int));
    int array_size = comm_size;
    int array_sum = 0;
    
    if(my_rank==0)  {
        MPI_Gather(num_array, array_size, MPI_INT, sum_array, comm_size, MPI_INT, 0, MPI_COMM_WORLD );
        array_sum += reduce_sequential(num_array, &array_size);
    }
    printf("Sum of Array is : %d", array_sum);
    
    free(num_array);
    MPI_Finalize();
    return 0;
}