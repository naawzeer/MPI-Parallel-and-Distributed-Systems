#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>


int main(int *argc, char **argv){
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);  

    MPI_Comm cart_comm;
    int reorder = 1;
    int dims[2] ={0,0};
    int coord[2] = {0,0};
    int periods[2] = {0,0};
    MPI_Dims_create(size, 2, dims);
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cart_comm);
    MPI_Cart_coords(cart_comm, rank, 2, coord);
    printf("My name is khan Rank %d is at coordinates (%d, %d)\n", rank, coord[0], coord[1]);



    MPI_Finalize();
    return 0;
}