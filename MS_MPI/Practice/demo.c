#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main() {

    int bins, beads;
    printf("Enter the number of beads you want to throw: ");
    scanf("%d", &beads);
    printf("Enter the number of bins you want to have : ");
    scanf("%d", &bins);
    
    printf("Beans : %d & Bins : %d \n", beads, bins);
    
    int *bins_array = (int *)calloc(bins, sizeof(int));
    
    for (int i = 0; i < bins; i++)
    {
        bins_array[i] = 0;
    }
    
    for (int i = 0; i < beads; i++)
    {
        int bin_position = bins/2;
        for (int j = 0; j < bins -1 ; j++)
        {
            int temp = rand() % 2;
            if (temp==0 && bin_position < bins - 1) {
                bin_position++;
            }
            else if (temp != 0 && bin_position > 0) {
                bin_position--;
            }           
        }
        bins_array[bin_position]++;
    }
        
    printf("Bins with beads in it : ");
    for (int i = 0; i < bins; i++)
    {
        printf("| %d |",bins_array[i]);
    }
    printf("\n");
int max_height = 0;
for (int i = 0; i < bins; i++) {
    if (bins_array[i] > max_height)
        max_height = bins_array[i];
}
printf("\nGalton Board (horizontal histogram / vertical bins):\n\n");
for (int row = max_height; row > 0; row--) {
    for (int col = 0; col < bins; col++) {
        if (bins_array[col] >= row) {
            printf("  *  ");
        } else {
            printf("     ");
        }
    }
    printf("\n");
}
for (int col = 0; col < bins; col++) {
    printf("=====");
}
printf("\n");
for (int col = 0; col < bins; col++) {
    printf(" %2d  ", col);
}
printf("\n");

    
    return 0;
}
