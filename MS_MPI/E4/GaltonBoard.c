#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Process %d of %d: Initialized MPI\n", rank, size);

    int bead_count = 0, bin_count = 0;

    // Synchronize processes here to debug input phase
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        printf("Enter Bead Count: ");
        fflush(stdout); // Ensure prompt is printed
        if (scanf("%d", &bead_count) != 1) {
            fprintf(stderr, "Failed to read bead count\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        printf("Enter Bin Count: ");
        fflush(stdout); // Ensure prompt is printed
        if (scanf("%d", &bin_count) != 1) {
            fprintf(stderr, "Failed to read bin count\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        printf("Master Process %d: Read Bead Count = %d, Bin Count = %d\n", rank, bead_count, bin_count);
    }

    // Synchronize before broadcasting
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&bead_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&bin_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD); // Synchronize after broadcasting

    printf("Process %d of %d: Bead Count = %d, Bin Count = %d\n", rank, size, bead_count, bin_count);

    int my_bead_count = bead_count / size;
    int remainder = bead_count % size;
    if (rank < remainder) {
        my_bead_count++;
    }

    printf("Process %d of %d: My Bead Count = %d\n", rank, size, my_bead_count);

    int *my_histogram = (int*)calloc(bin_count, sizeof(int));
    if (my_histogram == NULL) {
        fprintf(stderr, "Memory allocation failed for process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    srand(time(NULL) + rank);

    for (int bead = 0; bead < my_bead_count; bead++) {
        double pos = bin_count / 2.0 - 0.5;
        for (int height = 0; height < bin_count - 1; height++) {
            pos += ((double)rand() / RAND_MAX) - 0.5;
        }
        int index = (int)pos;
        if (index >= 0 && index < bin_count) {
            my_histogram[index]++;
        }
    }

    int *total_histogram = NULL;
    if (rank == 0) {
        total_histogram = (int*)calloc(bin_count, sizeof(int));
        if (total_histogram == NULL) {
            fprintf(stderr, "Memory allocation failed on master process\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    printf("Process %d of %d: Reducing histograms\n", rank, size);
    MPI_Reduce(my_histogram, total_histogram, bin_count, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total Histogram:\n");
        for (int i = 0; i < bin_count; i++) {
            printf("Bin %d: %d\n", i, total_histogram[i]);
        }
        free(total_histogram);
    }

    free(my_histogram);

    printf("Process %d of %d: Finalizing MPI\n", rank, size);
    MPI_Finalize();
    return 0;
}
