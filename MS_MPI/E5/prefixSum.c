#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>  
#include <stdbool.h>


void prefix_sequential(int* in, int in_length, int** outp)
{
	int* out = (int*)malloc(in_length*sizeof(int));

	out[0] = 0;
	for(int i=1; i < in_length; i++)
		out[i] = out[i-1] + in[i-1];
	
	*outp = out;
}

void prefix_mpi_gather_scatter(int* block_array, int block_size, int* block_prefix, MPI_Comm communicator)
{
    int my_rank;
    int com_size;
    MPI_Comm_rank(communicator, &my_rank);
    MPI_Comm_size(communicator, &com_size);

    int local_block_sum = 0;
    for (int i = 0; i < block_size; i++)
        local_block_sum += block_array[i];

    int* block_sums = NULL;
    int* block_sums_prefix = NULL;

    if (my_rank == 0)
    {
        block_sums = (int*)malloc(com_size * sizeof(int));
        block_sums_prefix = (int*)malloc(com_size * sizeof(int));
    }
	
    MPI_Gather(&local_block_sum, 1, MPI_INT, block_sums, 1, MPI_INT, 0, communicator);

    if (my_rank == 0)
    {
        block_sums_prefix[0] = 0;
        for (int p = 1; p < com_size; p++)
            block_sums_prefix[p] = block_sums[p-1] + block_sums_prefix[p-1];
    }

    MPI_Scatter(block_sums_prefix, 1, MPI_INT, block_prefix, 1, MPI_INT, 0, communicator);

    for (int i = 1; i < block_size; i++)
        block_prefix[i] = block_prefix[i-1] + block_array[i-1];


    if (my_rank == 0)
    {
        free(block_sums);
        free(block_sums_prefix);
    }
}


void prefix_mpi_allgather(int* block_array, int block_size, int* block_prefix, MPI_Comm communicator)
{
    int my_rank;
    int com_size;
    MPI_Comm_rank(communicator, &my_rank);
    MPI_Comm_size(communicator, &com_size);

    int local_block_sum = 0;
    for (int i = 0; i < block_size; i++)
        local_block_sum += block_array[i];

    int* block_sums = block_sums = (int*)malloc(com_size * sizeof(int));
    MPI_Allgather(&local_block_sum, 1, MPI_INT, block_sums, 1, MPI_INT, communicator);

	block_prefix[0] = 0;
	for (int p = 0; p < my_rank - 1; p++)
		block_prefix[0] += block_sums[p];

    for (int i = 1; i < block_size; i++)
        block_prefix[i] = block_prefix[i-1] + block_array[i-1];

	free(block_sums);
}


void prefix_mpi_scan(int* block_array, int block_size, int* block_prefix, MPI_Comm communicator)
{
    int my_rank;
    int com_size;
    MPI_Comm_rank(communicator, &my_rank);
    MPI_Comm_size(communicator, &com_size);

    int local_block_sum = 0;
    for (int i = 0; i < block_size; i++)
        local_block_sum += block_array[i];

    MPI_Scan(&local_block_sum, block_prefix, 1, MPI_INT, MPI_SUM, communicator);

	block_prefix[0] = block_prefix[0] - local_block_sum;

    for (int i = 1; i < block_size; i++)
        block_prefix[i] = block_prefix[i-1] + block_array[i-1];
}


int main(int argc, char** args)
{
    MPI_Init(&argc, &args);

    int my_rank;
    int com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);

    int total_array_size = 2048;

    if (total_array_size % com_size != 0)
        total_array_size = (total_array_size / com_size + 1) * com_size;


    int block_size = total_array_size / com_size;
    int* total_array = NULL;
    int* total_prefix = NULL;

    if (my_rank == 0)
    {
        total_array = (int*)malloc(total_array_size * sizeof(int));
        total_prefix = (int*)malloc(total_array_size * sizeof(int));
        for (int i = 0; i < total_array_size; i++)
            total_array[i] = rand() % 11;
    }

    int* block_array = (int*)malloc(block_size * sizeof(int));
    int* block_prefix = (int*)malloc(block_size * sizeof(int));

    MPI_Scatter(total_array, block_size, MPI_INT,
        block_array, block_size, MPI_INT, 0, MPI_COMM_WORLD);

    prefix_mpi_gather_scatter(block_array, block_size, block_prefix, MPI_COMM_WORLD);

    MPI_Gather(block_prefix, block_size, MPI_INT,
        total_prefix, block_size, MPI_INT, 0, MPI_COMM_WORLD);

    int accum = 0;
    if (my_rank == 0)
    {
        for (int i = 1; i < total_array_size; i++)
        {   
            
            accum += total_array[i - 1];
            if (total_prefix[i] != accum)
                printf("Error at index %i: %i expected, %i computed\n", i, accum, total_prefix[i]);
        }

        printf("Test completed!\n"); 
        free(total_array);
        free(total_prefix);
    }
    free(block_array);
    free(block_prefix);

    MPI_Finalize();
    
    return 0;
}
