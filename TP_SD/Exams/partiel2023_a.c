#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/* IMPLÉMENTATION : Comment faire pour diffuser un message entre les entités de calcul d'id pairs
* d'une part et les entités de calculs d'id impairs d'autre part. Donnez un exemple.
*/

/*
   Compilation : mpicc -Wall partiel2023_a.c -o partiel2023_a.exe
   Exécution   : mpirun -np 7 ./partiel2023_a.exe
*/


int main(int argc, char** argv){
    int n, id_global;
    int msg = -1; // pour les entités qui ne génèrent rien avant Bcast
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    if(id_global == 0 || id_global == 1) {
        srand(time(NULL)+id_global);
        msg = rand()%100;
        printf("L'entité globale %d génère le message à diffsuer : %d\n", id_global, msg);
    }

    int couleur = id_global%2;
    int nv_id = (id_global - id_global%2) / 2;

    MPI_Comm nv_comm;

    MPI_Comm_split(MPI_COMM_WORLD, couleur, nv_id, &nv_comm);

    MPI_Bcast(&msg, 1, MPI_INT, 0, nv_comm);

    int id_local;
    MPI_Comm_rank(nv_comm, &id_local);

    printf("Je suis l'entité globale %d (id_local %d)). J'ai la valeur : %d\n", id_global, id_local, msg);

    MPI_Comm_free(&nv_comm);
    MPI_Finalize();
    return 0;
}