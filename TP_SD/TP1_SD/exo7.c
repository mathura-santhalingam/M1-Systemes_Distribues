#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

// IMPLÉMENTATION Transmission en Cascade Circulaire (P0 vers Pn-1 puis retour à P0)

/*  mpicc -Wall exo7.c -o exo7.exe
    mpirun -np nb_processus ./exo7.exe
*/


int main(int argc, char** argv) {
    int n, id;
    int msg;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // On vérifie qu'on a bien au moins 2 processus
    if (n < 2) {
        if (id == 0) {
            printf("Erreur : il faut au moins 2 processus (mpirun -np 2+)\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (id == 0) {
        msg = 100; // On initialise la valeur à envoyer
        MPI_Send(&msg, 1, MPI_INT, ((id + 1) % n), 0, MPI_COMM_WORLD); 
        // Il reçoit du DERNIER processus (n - 1)
        MPI_Recv(&msg, 1, MPI_INT, n - 1, 0, MPI_COMM_WORLD, &status); 
        printf("Processus 0 a reçu la valeur finale : %d de %d\n", msg, n - 1);
    } else {
        // Pas de boucle for : chaque processus ne fait ça qu'une seule fois
        MPI_Recv(&msg, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
        printf("Processus %d a recu la valeur %d du processus %d\n", id, msg, id - 1);
        // 2. Il envoie au suivant. L'astuce (id+1)%n permet de boucler vers 0 à la fin
        MPI_Send(&msg, 1, MPI_INT, ((id + 1) % n), 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}