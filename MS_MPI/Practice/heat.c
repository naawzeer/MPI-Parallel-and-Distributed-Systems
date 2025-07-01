#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


// Swap two double pointer arrays
void swap(double** a, double** b) {
    double* temp = *a;
    *a = *b;
    *b = temp;
}


int main(int argc, char** args)
{
// Sub task a)
// Here you have to insert the initialization of MPI and to determine the amount of
// processes, the block size and the rank.
    MPI_Init(&argc, &args);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int block_size = 512 * 1024 * 1024 / size;
    if(512*1024*1024 % size < rank) block_size++;
    

double delta_t = 0.02;
int grid_size = 512 * 1024 * 1024;
int num_time_steps = 3000;
double conductivity = 0.1;

double* T_k = (double*) malloc(sizeof(double) * grid_size);
double* T_kn = (double*) malloc(sizeof(double) * block_size);

for (int i = 0; i < block_size; i++) {
    T_k[i] = i;
}
for (int k = 0; k < num_time_steps; k++)
{
// Sub task c)
// Here you have to insert the synchronization of the borders of the blocks of
// adjacent processes
    for (int i = 0; i < block_size; i++)
    {
    int i_left = i != 0 ? i - 1 : 1 ;
    int i_right = i != block_size - 1 ? i + 1 : i - 1;
    double dTdt_i = conductivity * (- 2 * T_k[i] + T_k[i_left] + T_k[i_right]);
    T_kn[i] = T_k[i] + delta_t * dTdt_i;
    }
    swap(&T_k, &T_kn);
}

double T_average = 0;
for (int i = 0; i < grid_size; i++)
T_average += T_k[i];
T_average = T_average / grid_size;
// Sub task d)
// Here you have to sum up and send T_average to the master process.
int global_average = 0;
MPI_Reduce(&T_average, &global_average, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

// Sub task e)
// Here you have to insert an if-condition, so that only the master process prints the
// average temperature on the console.
if (rank == 0){
    printf("T_average: %f", T_average);
}

// Sub task f)
// Here the finalisation of MPI is missing!
MPI_Finalize();
return 0;
}
