#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void swap(double** a, double** b) {
    double* temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Simulation parameters
    double delta_t = 0.02;
    int grid_size_x = 1024;
    int grid_size_y = 1024;
    int num_time_steps = 3000;
    double conductivity = 0.1;

    // Determine the number of processes in each dimension
    int dims[2] = {0, 0}; // Let MPI decide the dimensions
    MPI_Dims_create(size, 2, dims);

    // Define periodicity (non-periodic)
    int periods[2] = {0, 0};
    MPI_Comm comm_cart;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm_cart);

    // Get the Cartesian coordinates of the current process
    int coords[2];
    MPI_Cart_coords(comm_cart, rank, 2, coords);

    // Determine the block size for each process
    int bs_x = grid_size_x / dims[0];
    int bs_y = grid_size_y / dims[1];

    // Allocate memory for the local sub-grids
    double* T_k = (double*)malloc(sizeof(double) * bs_x * bs_y);
    double* T_kn = (double*)malloc(sizeof(double) * bs_x * bs_y);

    // Initialize the sub-grid with some values
    for (int y = 0; y < bs_y; y++) {
        for (int x = 0; x < bs_x; x++) {
            T_k[y * bs_x + x] = x + y + coords[0] * bs_x + coords[1] * bs_y;
        }
    }

    // Determine the ranks of neighboring processes
    int left, right, up, down;
    MPI_Cart_shift(comm_cart, 0, 1, &left, &right);
    MPI_Cart_shift(comm_cart, 1, 1, &down, &up);

    // Time-stepping loop
    for (int k = 0; k < num_time_steps; k++) {
        // Communicate borders with neighboring processes
        MPI_Request requests[8];
        MPI_Isend(T_k, bs_x, MPI_DOUBLE, up, 0, comm_cart, &requests[0]);
        MPI_Irecv(T_k + bs_y * bs_x, bs_x, MPI_DOUBLE, down, 0, comm_cart, &requests[1]);
        MPI_Isend(T_k + (bs_y - 1) * bs_x, bs_x, MPI_DOUBLE, down, 0, comm_cart, &requests[2]);
        MPI_Irecv(T_k - bs_x, bs_x, MPI_DOUBLE, up, 0, comm_cart, &requests[3]);

        // Create derived data type for left and right communication
        MPI_Datatype column_type;
        MPI_Type_vector(bs_y, 1, bs_x, MPI_DOUBLE, &column_type);
        MPI_Type_commit(&column_type);

        MPI_Isend(T_k, 1, column_type, left, 0, comm_cart, &requests[4]);
        MPI_Irecv(T_k + bs_x - 1, 1, column_type, right, 0, comm_cart, &requests[5]);
        MPI_Isend(T_k + bs_x - 1, 1, column_type, right, 0, comm_cart, &requests[6]);
        MPI_Irecv(T_k, 1, column_type, left, 0, comm_cart, &requests[7]);

        MPI_Waitall(8, requests, MPI_STATUSES_IGNORE);
        MPI_Type_free(&column_type);

        // Compute the heat equation for the local sub-grid
        for (int y = 1; y < bs_y - 1; y++) {
            for (int x = 1; x < bs_x - 1; x++) {
                int i = y * bs_x + x;
                int i_left = i - 1;
                int i_right = i + 1;
                int i_down = i - bs_x;
                int i_up = i + bs_x;

                double dTdt_i = conductivity * (-4 * T_k[i] +
                    T_k[i_left] + T_k[i_right] + T_k[i_down] + T_k[i_up]);
                T_kn[i] = T_k[i] + delta_t * dTdt_i;
            }
        }

        // Swap the grids
        swap(&T_k, &T_kn);
    }

    // Gather the results at the root process
    double* T_global = NULL;
    if (rank == 0) {
        T_global = (double*)malloc(sizeof(double) * grid_size_x * grid_size_y);
    }
    MPI_Gather(T_k, bs_x * bs_y, MPI_DOUBLE, T_global, bs_x * bs_y, MPI_DOUBLE, 0, comm_cart);

    // Compute the average temperature
    if (rank == 0) {
        double T_average = 0.0;
        for (int i = 0; i < grid_size_x * grid_size_y; i++) {
            T_average += T_global[i];
        }
        T_average /= (grid_size_x * grid_size_y);
        printf("T_average: %f \n", T_average);
        free(T_global);
    }

    // Free allocated memory
    free(T_k);
    free(T_kn);

    MPI_Finalize();
    return 0;
}

