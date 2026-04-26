#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"
#include <unistd.h>

// IMPLÉMENTATION DE L'ALGO SPANTREE(r) avec envoi et réception d'acc_recep

/*  mpicc -Wall exo5.c -o exo5.exe
    mpirun -np 8 ./exo5.exe

    N.B : pour mieux voir l'inondation, j'ai utilisé "usleep(50000);" de la bibliothèque "unistd.h"
*/

int main(int argc, char **argv) {
    // Variables
    int n, id, i;
    int *msg;
    int taille_msg;
    int acc_recep = 1;
    int k = 0;
    MPI_Status status;

    // Initialisation
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n < 2) {
        if (id == 0) printf("Erreur : il faut au moins 2 processus.\n");
        MPI_Finalize();
        return 1;
    }

    if (id == 0) {
        // Génération d'un nb aléat de val aléatoires
        srand(time(NULL));
        taille_msg = (rand() % 10) + 1;  // +1 car malloc(0) => erreur
        
        // Allocation de mémoire + remplissage de msg
        msg = (int *)malloc(taille_msg * sizeof(int));
        for (i = 0; i < taille_msg; i++) {
            msg[i] = rand() % 100;
        }
        
        // Affichage des valeurs générées :
        printf("Processus 0 lance l'inondation ( Taille: %d, msg: ", taille_msg);
        for (i = 0; i < taille_msg; i++) {
            printf("%d ", msg[i]);
        }
        printf(")\n");

        // Envoi : le noeud 0 envoie à tlm donc plsrs envois donc for
        for (i = 1; i < n; i++) {
            MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
            usleep(50000);
        }

        // Il a envoyé n-1 msg donc il attend n-1 acc_recep
        // On sonde pour connaitre le message qui arrive avec balise en balise 0 (le msg d'inondation)
        // Pcq 0 peut recevoir acc_recep comme il peut recevoir msg (ANY_TAG)
        for (i = 0; i < n - 1; i++) {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int count;
            MPI_Get_count(&status, MPI_INT, &count);
            int *temp = (int *)malloc(count * sizeof(int));
            // Comme on sait pas si c'est msg ou acc_recep on met dans temp pour chercher sa balise
            MPI_Recv(temp, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            // Si c'est un acc_recep
            if (status.MPI_TAG == 1) {
                k++;
            }
            free(temp);
        }
    } 
    else {
        // Reception de taille du msg
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status); // sonde
        MPI_Get_count(&status, MPI_INT, &taille_msg); // recup taille exacte
        
        // Allocation mém
        msg = (int *)malloc(taille_msg * sizeof(int));
        
        // Reception du msg
        MPI_Recv(msg, taille_msg, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
        
        // ATTENTION : On SAUVEGARDE le parent avant que status ne soit écrasé
        int parent = status.MPI_SOURCE;

        MPI_Send(&acc_recep, 1, MPI_INT, parent, 1, MPI_COMM_WORLD);

        // Env à tous les voisins sauf expediteur et lui-m
        for (i = 0; i < n; i++) {
            if (i != parent && i != id) {
                MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        free(msg);

        // Il a envoyé msg a ses n-2 voisins, donc att n-2 acc_recep
        for (i = 0; i < n - 2; i++) {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int count;
            MPI_Get_count(&status, MPI_INT, &count);
            int *temp = (int *)malloc(count * sizeof(int));
            
            MPI_Recv(temp, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            
            if (status.MPI_TAG == 1) {
                k++;
            }
            free(temp);
        }
    }

    // Affichage
    if (k == 0) {
        printf("Processus %d affiche : feuille\n", id);
    } else {
        printf("Processus %d affiche : parent de %d feuilles\n", id, k);
    }

    MPI_Finalize();
    return 0;
}
/* MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
Les messages sont received quelque soit leur tag dans leur ordre de sent,
donc si le processus 1 send à 0 un acc_recep de balise 1
puis que le processus 2 send à 0 un msg de balise 0
puis que le processus 3 send à 0 un acc_recep de balise 1;
0 va devoir lire les receptions dans cet ordre
et donc si on met TAG=1 dans MPI_Probe, il va se bloquer à la reception du msg de 2.
D'où "ANY_TAG" dans MPI_Probe */

/* MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
Dans else, on sait que la premiere reception sera un msg et non un acc_recep
(car un processus n'envoie un accusé de réception (tag=1) que s'il a déjà reçu un message d'inondation (tag=0))
mais on sait pas de qui.*/