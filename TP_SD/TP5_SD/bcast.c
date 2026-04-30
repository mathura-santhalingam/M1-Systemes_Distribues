#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main()
{
    int size, rank, message;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank==0) message=42;
    
    MPI_Bcast(&message, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}