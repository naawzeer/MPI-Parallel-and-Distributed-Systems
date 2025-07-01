#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void simulateGaltonBoard(int num_beads, int num_bins, int* histogram) {
    for (int i = 0; i < num_beads; i++) {
        int position = 0;
        for (int j = 0; j < num_bins - 1; j++) {
            if (rand() % 2 == 0) {
                position++; // Move right
            }
            // If rand() % 2 == 1, stay in the same position (move left)
        }
        histogram[position]++; // Increment the count for the final bin
    }
}

void printHistogram(int num_bins, int* histogram) {
    printf("\nGalton Board Results:\n");
    for (int i = 0; i < num_bins; i++) {
        printf("Bin %d: %d\n", i, histogram[i]);
    }
}

int main() {
    int num_beads, num_bins;

    // Ask the user for input
    printf("Enter the amount of beads to be traced: ");
    scanf("%d", &num_beads);
    printf("Enter the amount of bins: ");
    scanf("%d", &num_bins);

    // Seed the random number generator
    srand(time(NULL));

    // Allocate memory for the histogram
    int* histogram = (int*)calloc(num_bins, sizeof(int));
    if (histogram == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Run the simulation
    simulateGaltonBoard(num_beads, num_bins, histogram);

    // Print the results
    printHistogram(num_bins, histogram);

    // Free allocated memory
    free(histogram);

    return 0;
}

