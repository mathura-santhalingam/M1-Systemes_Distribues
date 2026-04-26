#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"
#include <unistd.h> // Pour le usleep()

// IMPLÉMENTATION DE L'ALGO FLOOD_ECHO(r)

/*  mpicc -Wall flood_echo_v1.c -o flood_echo_v1.exe
    mpirun -np 8 ./flood_echo_v1.exe
*/

int main(int argc, char** argv) {
    int n, id, i;
    int m;      // Représente 'm' ou 'k' dans le couple (m, b)
    int c;      // Compteur d'attente
    int parent = -1;
    
    int nb_enfants = 0;
    int *enfants; 
    
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n < 2) {
        if (id == 0) printf("Erreur: il faut au moins 2 processus.\n");
        MPI_Finalize();
        return 1;
    }

    // Allocation pour le tableau_enfants
    enfants = (int *)malloc(n * sizeof(int));

    // Si i == r alors
    if (id == 0) { 
        srand(time(NULL));
        int k = rand() % 100; // k est la donnée générée
        m = k; 
        
        printf("Processus 0 lance l'inondation avec la donnee : %d\n", k);

        // Env (k, 0) à tous les voisins (de r)
        for (i = 1; i < n; i++) {
            // m = k, balise (TAG) = 0
            MPI_Send(&m, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
            usleep(50000); // Ralentissement pour voir l'inondation
        }
        
        // Poser c <- 2(# voisins)  // c sera le nombre de reception à faire, or dans un graphe complet, r a (n-1) voisins
        c = 2 * (n - 1);
    } else { 
        // Rec (k, 0) d'un voisin père (inconnu)
        // On force la balise à 0 pour être sûr de capter la première inondation
        MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        parent = status.MPI_SOURCE;
        
        int k = m; // On sauvegarde la donnée reçue

        // Env (k, 0) à tous les voisins (de i) sauf père
        for (i = 0; i < n; i++) {
            if (i != id && i != parent) {
                // m = k, balise (TAG) = 0
                MPI_Send(&k, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        
        // Poser c <- 2(# voisins) - 2
        c = 2 * (n - 1) - 2; // ou c = 2 * n - 4;
    }

    while (c > 0) {
        // Rec (m, b) d'un voisin j (inconnu)
        MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int j = status.MPI_SOURCE;
        int b = status.MPI_TAG; // b correspond à la balise (TAG) de MPI

        if (b == 1) {
            if (m == 1) {
                // Ajouter j au tableau_enfants
                enfants[nb_enfants] = j;
                nb_enfants++;
                // Màj du compteur
                c = c - 1;
            }
            
            // Màj du compteur
            c = c - 1; 
        }

        if (b == 0) {
            // Env (0, 1) à j
            int ack_m = 0; 
            // On envoie ack_m = 0, avec la balise = 1, au processus j
            MPI_Send(&ack_m, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
            
            // Màj du compteur
            c = c - 1;
        }
    }

    if (id != 0) {
        // Env (1, 1) à père
        int msg_pere_m = 1;
        // On envoie msg_pere_m = 1, avec la balise = 1, au processus parent
        MPI_Send(&msg_pere_m, 1, MPI_INT, parent, 1, MPI_COMM_WORLD);
    }

    // Fin de l'algorithme - Affichage de contrôle
    MPI_Barrier(MPI_COMM_WORLD); // Synchronisation pour que l'affichage soit propre
    
    printf("Processus %d a %d enfants : ", id, nb_enfants);
    for (i = 0; i < nb_enfants; i++) {
        printf("%d ", enfants[i]);
    }
    printf("\n");

    free(enfants);
    MPI_Finalize();
    return 0;
}