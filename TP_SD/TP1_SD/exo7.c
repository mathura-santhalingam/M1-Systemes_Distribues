#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char** argv) {
    int n, id; // Plus besoin du i
    int val_a_env, msg_recu;
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
        val_a_env = 100; // On initialise la valeur à envoyer
        MPI_Send(&val_a_env, 1, MPI_INT, ((id+1)%n), 0, MPI_COMM_WORLD); 
        // Il reçoit du DERNIER processus (n - 1)
        MPI_Recv(&msg_recu, 1, MPI_INT, n - 1, 0, MPI_COMM_WORLD, &status); 
        printf("Processus 0 a reçu la valeur finale : %d\n", msg_recu);
    } else {
        // Pas de boucle for : chaque processus ne fait ça qu'une seule fois
        MPI_Recv(&msg_recu, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
        printf("Processus %d a recu la valeur %d\n", id, msg_recu);
        // 2. Il envoie au suivant. L'astuce (id+1)%n permet de boucler vers 0 à la fin
        MPI_Send(&msg_recu, 1, MPI_INT, (id + 1) % n, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}