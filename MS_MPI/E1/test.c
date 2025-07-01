#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to compute exclusive prefix sum in parallel
void prefix_parallel(int* in, int in_length, int** outp) {
    int* out = (int*)malloc(in_length * sizeof(int));
    int num_threads;
    
    // Step 1: Compute block sums in parallel
    #pragma omp parallel
    {
        num_threads = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        int start = (in_length / num_threads) * thread_id;
        int end = (thread_id == num_threads - 1) ? in_length : start + (in_length / num_threads);
        
        int sum = 0;
        for (int i = start; i < end; i++) {
            sum += in[i];
        }
        
        out[start] = (thread_id == 0) ? 0 : sum; // Start of each block in out is the sum of the block
        for (int i = start + 1; i < end; i++) {
            out[i] = out[i - 1] + in[i - 1];
        }
    }
    
    // Step 2: Compute prefix sum of block sums sequentially
    for (int i = 1; i < num_threads; i++) {
        int start = (in_length / num_threads) * i;
        int end = (i == num_threads - 1) ? in_length : start + (in_length / num_threads);
        for (int j = start; j < end; j++) {
            out[j] += out[start - 1];
        }
    }
    
    *outp = out;
}

int main() {
    int in_length = 16;
    int in[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int* out;

    prefix_parallel(in, in_length, &out);

    printf("Input: ");
    for (int i = 0; i < in_length; i++) {
        printf("%d ", in[i]);
    }
    printf("\n");

    printf("Exclusive Prefix Sum: ");
    for (int i = 0; i < in_length; i++) {
        printf("%d ", out[i]);
    }
    printf("\n");

    free(out);
    return 0;
}

