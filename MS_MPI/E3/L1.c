#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    int rank, size;
    int send_data, data, recv_data, local_value, global_sum;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    local_value = rank+1;
    MPI_Reduce(&local_value, &global_sum, 1, MPI_INT, MPI_BAND, 0, MPI_COMM_WORLD);
    printf("Rank : %d, Local = %d, Globel = %d\n ", rank, local_value, global_sum);
    if (rank==0) {
        printf("Total sum of ranks = %d\n", global_sum);
        
    
    }MPI_Finalize();
    return 0;
}




////////////////////////////////////////////////
/////////// BroadCast PROGRAM //////////////
////////////////////////////////////////////////
    // if (rank==0){
    //     data = 100;
    //     printf("Proces : %d is brodcasting data : %d\n", rank, data);
    // }

    // MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // printf("Process : %d has received broadcasted data data : %d\n", rank, data);


////////////////////////////////////////////////
/////////// P2P SEND RECV PROGRAM //////////////
////////////////////////////////////////////////
    // // Ensure there are at least 2 processes
    // if (size < 2) {
    //     if (rank == 0) {
    //         printf("This program requires at least 2 processes.\n");
    //     }
    //     MPI_Finalize();
    //     return 1;
    // }

    // int dest = (rank + 1) % size;          // Destination process (next process)
    // int source = (rank - 1 + size) % size;   // Source process (previous process)

    // // Each process sets its send_data to its own rank.
    // send_data = rank+20;
    // printf("Iteration send = %d receive = %d\n", send_data, recv_data);
    // // Use MPI_Sendrecv to send and receive data in one combined call.
    // MPI_Sendrecv(
    //     &send_data, 1, MPI_INT,   // Send buffer, number of elements, and data type
    //     0, 0,                  // Destination rank and send tag (here, tag 0)
    //     &recv_data, 1, MPI_INT,   // Receive buffer, number of elements, and data type
    //     source, 0,                // Source rank and receive tag (should match sender's tag)
    //     MPI_COMM_WORLD,           // Communicator
    //     MPI_STATUS_IGNORE         // Status (ignored in this example)
    // );

    // // Each process prints what it sent and what it received.
    // printf("Process %d sent data %d to process %d and received data %d from process %d\n",
    //        rank, send_data, dest, recv_data, source);

    // if (rank == 0){
    //     MPI_Send(&message, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);
    //     printf("Rank 0 has send message : %d to Rank : 1\n", message);
    // }
    // else if(rank==1) {
    //     MPI_Recv(&receive, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    //     printf("Rank 1 has received message : %d from Rank : 0\n", receive);
    // }

////////////////////////////////////////////////
//////////HELLO WORLD PROGRAM///////////////////
////////////////////////////////////////////////
// int main(int argc, char **argv) {

//     MPI_Init(&argc, &argv);
//     int rank, size;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);

//     printf("Rank : %d has Size : %d.\n", rank, size);
//     MPI_Finalize();
//     return 0;

// }