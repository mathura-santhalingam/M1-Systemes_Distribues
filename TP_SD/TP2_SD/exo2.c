#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"
#include <unistd.h>

// IMPLÉMENTATION DE L'ALGO FLOOD(r)

/*  mpicc -Wall exo2.c -o exo2
    mpirun -np 2 ./exo2 => erreur
    mpirun -np 8 ./exo2

    N.B : on ne voyait pas l'inondation car le processus 0 est trop rapide :
    il réussit à envoyer à tous ses voisins avant que quique ce soit n'envoie à son tour pour le devancer.
    Donc j'ai mis un "usleep(50000);" (#include <unistd.h>) pour un peu plus voir l'effet de l'inondation.
*/

int main(int argc, char** argv) {
    int n, id, i;
    int msg[2];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n < 2) {
        if (id == 0) {
            printf("Erreur: il faut au moins 2 processus.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (id == 0) {
        // Génération uniquement par l'initiateur
        srand(time(NULL));
        msg[0] = rand() % 100;
        msg[1] = rand() % 100;
        printf("Processus 0 lance l'inondation avec [%d, %d]\n", msg[0], msg[1]);

        // Envoi à tous les autres (graphe complet)
        for (i = 1; i < n; i++) {
            MPI_Send(msg, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            usleep(50000);
        }
    } 
    else {
        // Réception de n'importe qui
        MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        // Affichage demandé par l'énoncé
        printf("Processus %d a recu [%d, %d] du processus %d\n", id, msg[0], msg[1], status.MPI_SOURCE);

        // Envoi à tous les voisins sauf l'expéditeur ET sauf soi-même
        for (i = 0; i < n; i++) {
            if (i != status.MPI_SOURCE && i != id) {
                MPI_Send(msg, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Finalize();
    return 0;
}