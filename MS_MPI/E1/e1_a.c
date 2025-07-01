#include <stdio.h>

int main() {
    int num_beads, num_bins;

    // Ask the user to enter the number of beads
    printf("Enter the amount of beads to be traced: ");
    scanf("%d", &num_beads);

    // Ask the user to enter the number of bins
    printf("Enter the amount of bins: ");
    scanf("%d", &num_bins);

    // Display the entered values
    printf("\nYou entered:\n");
    printf("Number of beads: %d\n", num_beads);
    printf("Number of bins: %d\n", num_bins);

    return 0;
}

