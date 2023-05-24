#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char** argv){
    int process_Rank, size_Of_Cluster;
    char hostname[256];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    gethostname(hostname,255);

    printf("Hello World from process %d of %d on host %s\n", process_Rank, size_Of_Cluster, hostname);

    MPI_Finalize();
    return 0;
}
