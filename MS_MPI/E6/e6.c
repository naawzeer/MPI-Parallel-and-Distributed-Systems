#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Function to simulate the Galton Board for a given number of beads
void simulate_galton_board(int beads, int bins, int *histogram) {
    srand(time(NULL) + MPI_Wtime()); // Seed the random number generator
    for (int i = 0; i < beads; i++) {
        int position = 0;
        for (int j = 0; j < bins - 1; j++) {
            position += rand() % 2; // Randomly move left or right
        }
        histogram[position]++;
    }
}

int main(int argc, char **argv) {
    int rank, size;
    int beads, bins;
    int *local_histogram, *global_histogram;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Master process queries parameters
    if (rank == 0) {
        printf("Enter the number of beads: ");
        scanf("%d", &beads);
        printf("Enter the number of bins: ");
        scanf("%d", &bins);
    }

    // Broadcast parameters to all processes
    MPI_Bcast(&beads, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&bins, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for histograms
    local_histogram = (int *)calloc(bins, sizeof(int));
    global_histogram = (int *)calloc(bins, sizeof(int));

    // Divide beads among processes
    int local_beads = beads / size;
    if (rank < beads % size) {
        local_beads++; // Distribute remainder beads
    }

    // Simulate Galton Board for local beads
    simulate_galton_board(local_beads, bins, local_histogram);

    // Reduce local histograms into global histogram
    MPI_Reduce(local_histogram, global_histogram, bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Master process prints the result
    if (rank == 0) {
        printf("Final Histogram:\n");
        for (int i = 0; i < bins; i++) {
            printf("Bin %d: %d\n", i, global_histogram[i]);
        }
    }

    // Free memory and finalize MPI
    free(local_histogram);
    free(global_histogram);
    MPI_Finalize();

    return 0;
}
