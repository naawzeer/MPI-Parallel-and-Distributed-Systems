#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char** argv) {
    int rank, size, beads, bins;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(time(NULL) + rank);  // Unique seed for each process
    if (rank == 0) {
        printf("Enter number of beads : ");
        fflush(stdout);
        scanf("%d", &beads);
        printf("Enter number of bins : ");
        fflush(stdout);
        scanf("%d", &bins);
        printf("Number of beads: %d \n", beads);
        printf("Number of bins: %d \n", bins);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Broadcast input values to all processes
    MPI_Bcast(&beads, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&bins, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Determine beads per process
    int beads_per_process = beads / size;
    int remainder = beads % size;
    if (rank < remainder) beads_per_process++;  // Distribute leftover beads

    // Allocate local bin array
    int* local_bins = (int*)calloc(bins, sizeof(int));

    // Simulate Galton board (classic: bin = number of right turns)
    for (int i = 0; i < beads_per_process; i++) {
        int bin_index = 0;
        for (int j = 0; j < bins - 1; j++) {
            int step = rand() % 2; // 0 = left, 1 = right
            if (step == 1) bin_index++;
        }
        local_bins[bin_index]++;
    }

    // Root will collect all data
    int* global_bins = NULL;
    if (rank == 0) {
        global_bins = (int*)calloc(bins, sizeof(int));
    }
    // Broadcast global_bins to all processes
    
    // Reduce all local bins into global bins
    MPI_Reduce(local_bins, global_bins, bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Root process prints the histogram
    if (rank == 0) {
        printf("\nGalton Board Histogram:\n");

        // Find max height for vertical display
        int max = 0;
        for (int i = 0; i < bins; i++)
            if (global_bins[i] > max) max = global_bins[i];

        for (int h = max; h > 0; h--) {
            for (int i = 0; i < bins; i++) {
                if (global_bins[i] >= h)
                    printf("  *  ");
                else
                    printf("     ");
            }
            printf("\n");
        }

        for (int i = 0; i < bins; i++) printf("=====");
        printf("\n");
        for (int i = 0; i < bins; i++) printf(" %2d ", i);
        printf("\n");

        free(global_bins);
    }

    free(local_bins);
    MPI_Finalize();
    return 0;
}
