#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"

// IMPLÉMENTATION DE L'ALGO FLOOD(r) avec envoie d'acc_recep

/*  mpicc -Wall exo4.c -o exo4.exe
    mpirun -np 8 ./exo4.exe
*/

int main(int argc, char** argv) {
    int n, id, i;
    int *msg; 
    int taille_msg;
    int acc_recep = 1; // L'accusé de réception
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
        taille_msg = (rand() % 10) + 1; // +1 car malloc(0) => erreur
        msg = (int *)malloc(taille_msg * sizeof(int));

        for (i = 0; i < taille_msg; i++) {
            msg[i] = rand() % 100;
        }
        // Affichage
        printf("Processus 0 lance l'inondation ( Taille: %d, msg: ", taille_msg);
        for (i = 0; i < taille_msg; i++) {
            printf("%d ", msg[i]);
        }
        printf(")\n");

        // Envoi avec la balise 0 (Message d'inondation)
        for (i = 1; i < n; i++) {
            MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        printf("En exécutant ce programme, on remarque que malgré l'envoi de l'accusé de réception, ");
        printf("rien ne change fondamentalement dans l'exécution ni dans les résultats affichés ");
        printf("car personne ne le réceptionne.");
    } 
    else {
        // On sonde pour connaitre le message qui arrive avec balise en balise 0 (le msg d'inondation)
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &taille_msg);
        
        msg = (int *)malloc(taille_msg * sizeof(int));

        // On reçoit le premier message d'inondation
        MPI_Recv(msg, taille_msg, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
        
        // On renvoie l'entier 1 à celui qui nous a envoyé le premier message avec la balise 1
        MPI_Send(&acc_recep, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
        printf("Le processus %d a recu un msg de taille %d du processus %d et lui a envoyé un accusé de réceprion.\n", id, taille_msg, status.MPI_SOURCE);
        // Relais de l'inondation aux autres (balise 0)
        for (i = 0; i < n; i++) {
            if (i != status.MPI_SOURCE && i != id) {
                MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    }

    free(msg);
    MPI_Finalize();
    return 0;
}