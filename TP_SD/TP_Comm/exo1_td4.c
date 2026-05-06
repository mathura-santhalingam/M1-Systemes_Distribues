#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// IMPLÉMENTATION : split en 3

/*
   Compilation : mpicc -Wall exo1_td4.c -o exo1_td4.exe
   Exécution   : mpirun -np 8 ./exo1_td4.exe
*/

int main (int argc, char** argv){
   int nb, id_global;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb);
   MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

   MPI_Comm comm_3;

   int couleur = id_global%3;
   int nv_id = (id_global - id_global%3) / 3;

   MPI_Comm_split(MPI_COMM_WORLD, couleur, nv_id, &comm_3);

   int nb_local, id_local;

   MPI_Comm_size(comm_3, &nb_local);
   MPI_Comm_rank(comm_3, &id_local);

   printf("Je suis le global %d, dans mon nouveau communicateur je suis le %d\n", id_global, id_local);

   MPI_Finalize();
   return 0;
}