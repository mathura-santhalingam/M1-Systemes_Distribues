#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/* IMPLÉMENTATION : Supposons n=8. On veut créer un communicateur de topologie cartésienne de dimension 2
* avec 4 entités de calculs dans la 1ère dimension, 1ère dimension étant périodique (mais pas la 2ème).
* Dans ce communicateur quel est le nombre de voisins de chaque entité ? */

/*
   Compilation : mpicc -Wall partiel2023_b.c -o partiel2023_b.exe
   Exécution   : mpirun -np 8 ./partiel2023_b.exe
*/


int main(int argc, char** argv){
    int n, id_global;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    // on s'assure que l'utilisateur lance bien 8 processus
    if (n != 8) {
        if (id_global == 0) {
            printf("Erreur : Ce programme nécessite exactement 8 processus (n=8).\n");
        }
        MPI_Finalize();
        return 1;
    }

    MPI_Comm nv_comm;
    int dims[2] = {4,2};
    int periodes[2] = {1,0};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodes, 1, &nv_comm);

    // Affichage pour vérifier la topologie (le voisinage)
    int coords[2];
    int id_local;
    MPI_Comm_rank(nv_comm, &id_local);
    MPI_Cart_coords(nv_comm, id_local, 2, coords);

    MPI_Barrier(MPI_COMM_WORLD);
    printf("Processus Global %d -> Rang Local %d | Coordonnées : (%d, %d)\n", id_global, id_local, coords[0], coords[1]);

    MPI_Finalize();
    return 0;
}