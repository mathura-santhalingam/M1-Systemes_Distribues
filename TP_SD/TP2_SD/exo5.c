#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"
#include <unistd.h>

/*  mpicc -Wall exo5.c -o exo5
    mpirun -np 2 ./exo5 => erreur
    mpirun -np 8 ./exo5

    N.B : pour mieux voir l'inondation, j'ai utilisé "usleep(50000);" de la bibliothèque "unistd.h"
*/

int main(int argc, char **argv) {
    // Variables
    int n, id, i;
    int *msg;
    int taille_msg;
    int acc = 1;
    int k = 0;
    MPI_Status status;

    // Initialisation
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Vérif # processus
    if (n < 2) {
        if (id == 0) printf("Erreur : il faut au moins 2 processus.\n");
        MPI_Finalize();
        return 1;
    }

    if (id == 0) {
        // Génération d'un nb aléat de val aléatoires
        srand(time(NULL));
        taille_msg = (rand() % 10) + 1;  // +1 car malloc(0) => erreur
        
        // Allocation de mémoire
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

        // Il a envoyé n-1 msg donc il atend n-1 acc-recept
        for (i = 0; i < n - 1; i++) {
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

        MPI_Send(&acc, 1, MPI_INT, parent, 1, MPI_COMM_WORLD);

        // Env à tous les voisins sauf expediteur et lui-m
        for (i = 0; i < n; i++) {
            if (i != parent && i != id) {
                MPI_Send(msg, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        free(msg);

        // Il a envoyé msg a ses n-2 voisins, donc att n-2 acc-recept
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