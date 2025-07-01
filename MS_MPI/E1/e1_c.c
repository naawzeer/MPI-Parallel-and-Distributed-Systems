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

void createAsciiImage(int num_bins, int* histogram, int scale) {
    // Determine the maximum height of the histogram after scaling
    int max_height = 0;
    for (int i = 0; i < num_bins; i++) {
        if (histogram[i] / scale > max_height) {
            max_height = histogram[i] / scale;
        }
    }

    // Print the ASCII image
    printf("\nASCII Image of the Histogram:\n");
    for (int row = max_height - 1; row >= 0; row--) {
        for (int bin = 0; bin < num_bins; bin++) {
            int scaled_height = histogram[bin] / scale;
            printf("%c ", (row < scaled_height) ? 'X' : ' ');
        }
        printf("\n");
    }
}

void printHistogram(int num_bins, int* histogram) {
    printf("\nHistogram Results:\n");
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

    // Print the histogram values
    printHistogram(num_bins, histogram);

    // Create and display an ASCII image of the histogram
    int scale;
    printf("\nEnter a scale factor (e.g., how many beads per 'X'): ");
    scanf("%d", &scale);
    
    if (scale <= 0) {
        printf("Invalid scale factor. Using default scale of 1.\n");
        scale = 1;
    }
    
    createAsciiImage(num_bins, histogram, scale);

    // Free allocated memory
    free(histogram);

    return 0;
}
    