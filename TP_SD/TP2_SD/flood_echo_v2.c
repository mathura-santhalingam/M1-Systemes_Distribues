#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"
#include <unistd.h>

// IMPLÉMENTATION DE L'ALGO FLOOD&ECHO(4) APPLIQUÉ À UN GRAPHE

/*  mpicc -Wall flood_echo_v2.c -o flood_echo_v2.exe
    mpirun -np 8 ./flood_echo_v2.exe
*/

int main(int argc, char** argv) {
    int n, id;
    int m, c;
    int parent = -1;
    int nb_enfants = 0;
    int enfants[6]; 
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n != 6) {
        if (id == 0) printf("Erreur: il faut 6 processus.\n");
        MPI_Finalize(); return 1;
    }

    // Topologie du graphe
    int degres[6] = {3, 2, 3, 4, 1, 1};
    int voisins[6][4] = {
        {1, 2, 3, -1}, {0, 3, -1, -1}, {0, 3, 5, -1}, 
        {0, 1, 2, 4}, {3, -1, -1, -1}, {2, -1, -1, -1}
    };
    int mon_degre = degres[id];

    // ---------------------------------------------------
    // 1. PHASE D'INITIATION ET DE DESCENTE
    // ---------------------------------------------------
    if (id == 4) { 
        srand(time(NULL));
        m = rand() % 100;
        printf("Racine 4 lance FLOOD & ECHO avec la donnee : %d\n", m);

        for (int i = 0; i < mon_degre; i++) {
            MPI_Send(&m, 1, MPI_INT, voisins[id][i], 0, MPI_COMM_WORLD); 
        }
        c = 2 * mon_degre; 
    } 
    else { 
        MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        parent = status.MPI_SOURCE;
        
        for (int i = 0; i < mon_degre; i++) {
            int voisin_actuel = voisins[id][i];
            if (voisin_actuel != parent) {
                MPI_Send(&m, 1, MPI_INT, voisin_actuel, 0, MPI_COMM_WORLD);
            }
        }
        c = 2 * (mon_degre - 1); // J'attends des réponses de tout le monde SAUF mon père
    }

    // ---------------------------------------------------
    // 2. BOUCLE D'ATTENTE ET DE RÉSOLUTION
    // ---------------------------------------------------
    while (c > 0) {
        MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int j = status.MPI_SOURCE;
        int b = status.MPI_TAG; 

        if (b == 1) { // Contrôle
            if (m == 1) { // Adoption
                enfants[nb_enfants] = j;
                nb_enfants++;
                c = c - 2;
            } else { // Accusé simple suite à un doublon qu'on a envoyé
                c = c - 1; 
            }
        }
        else if (b == 0) { // Inondation en retard (doublon reçu)
            int ack_m = 0; 
            MPI_Send(&ack_m, 1, MPI_INT, j, 1, MPI_COMM_WORLD); // On renvoie un râteau (0, 1)
            c = c - 1;
        }
    }

    // ---------------------------------------------------
    // 3. PHASE ECHO (REMONTÉE)
    // ---------------------------------------------------
    if (id != 4) {
        int msg_pere_m = 1;
        MPI_Send(&msg_pere_m, 1, MPI_INT, parent, 1, MPI_COMM_WORLD); // Envoi de (1, 1)
    }

    MPI_Barrier(MPI_COMM_WORLD); 
    printf("Processus %d a %d enfants : ", id, nb_enfants);
    for (int i = 0; i < nb_enfants; i++) { printf("%d ", enfants[i]); }
    printf("\n");

    MPI_Finalize();
    return 0;
}