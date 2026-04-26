#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"

// IMPLÉMENTATION DE L'ALGO FLOOD(r) avec taille du message généré aléatoirement (MPI_Probe)

/*  mpicc -Wall exo3.c -o exo3.exe
    mpirun -np 1 ./exo3.exe => erreur
    mpirun -np 8 ./exo3.exe
*/

int main(int argc, char** argv) {
    int n, id, i;
    int *msg; // Pointeur pour l'allocation dynamique
    int taille_msg;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n < 2) {
        if (id == 0) printf("Erreur : il faut au moins 2 processus.\n");
        MPI_Finalize();
        return 1;
    }

    if (id == 0) {
        srand(time(NULL));
        // Taille du msg aléatoire entre 1 et 10
        taille_msg = (rand() % 10) + 1;  // +1 car malloc(0) => erreur
        
        // Allocation dynamique pour le processus 0
        msg = (int *)malloc(taille_msg * sizeof(int));

        // Génération de taille_msg valeurs aléatoires dans msg
        for (i = 0; i < taille_msg; i++) {
            msg[i] = rand() % 100;
        }
        // Affichage de la taille et de toutes les valeurs générées :
        printf("Processus 0 lance l'inondation ( Taille: %d, msg: ", taille_msg);
        for (i = 0; i < taille_msg; i++) {
            printf("%d ", msg[i]);
        }
        printf(")\n");

        // Envoi
        for (i = 1; i < n; i++) {
            MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } 
    else {
        // On sonde pour connaitre le message qui arrive
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        // On récupère la taille exacte
        MPI_Get_count(&status, MPI_INT, &taille_msg);
        
        // ON ALLOUE LA MÉMOIRE en fonction de la taille exacte du msg reçu
        msg = (int *)malloc(taille_msg * sizeof(int));

        // On reçoit le message (en utilisant la même source que la source sondée par MPI_Probe)
        MPI_Recv(msg, taille_msg, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        printf("Le processus %d a recu un msg de taille %d du processus %d\n", id, taille_msg, status.MPI_SOURCE);

        // Relais à tous les autres
        for (i = 0; i < n; i++) {
            if (i != status.MPI_SOURCE && i != id) {
                MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    }

    // Libération de la mémoire
    free(msg);

    MPI_Finalize();
    return 0;
}