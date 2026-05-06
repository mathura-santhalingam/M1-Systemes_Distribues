#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/*
   Compilation : mpicc -Wall reduce.c -o reduce.exe
   Exécution   : mpirun -np 4 ./reduce.exe
*/

int main(int argc, char** argv) {
    int id, nb_processus;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus);

    // Initialisation de l'aléatoire pour chaque processus
    srand(time(NULL) + id); 

    // 1. PRÉPARATION DES DONNÉES LOCALES (Le tableau de 2 entiers)
    int stats_joueur[2];
    
    stats_joueur[0] = rand() % 500; // Score du joueur (entre 0 et 499)
    stats_joueur[1] = rand() % 20;  // Ennemis vaincus (entre 0 et 19)

    printf("Joueur %d a terminé : [Score: %d | Ennemis: %d]\n", id, stats_joueur[0], stats_joueur[1]);

    // On prépare le tableau qui va recevoir le grand total (utile que pour id 0)
    int total_equipe[2] = {0, 0}; 

    MPI_Barrier(MPI_COMM_WORLD); 

    // 2. LA RÉDUCTION (Additionner les tableaux)
    // stats_joueur : Ce que j'envoie
    // total_equipe : Ce que la racine (0) reçoit
    // MPI_SUM      : On additionne les indices 0 entre eux, et les indices 1 entre eux.
    
    MPI_Reduce(stats_joueur, total_equipe, 2, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // 3. AFFICHAGE DU RÉSULTAT PAR LA RACINE
    if (id == 0) {
        printf("BILAN DE L'ÉQUIPE (Calculé par l'entité 0)\n");
        printf("-> Score total de l'équipe : %d\n", total_equipe[0]);
        printf("-> Total ennemis vaincus   : %d\n", total_equipe[1]);
    }

    MPI_Finalize();
    return 0;
}

/*EXEMPLE MATRICE :

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 8 // Taille de la matrice globale (8x8)


int main(int argc, char** argv) {
    int id, p;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (N % p != 0) {
        if (id == 0) printf("Erreur: N doit être divisible par p.\n");
        MPI_Finalize();
        return 1;
    }

    // 1. CONSTRUCTION DE LA MATRICE (Chaque entité gère k lignes)
    int k = N / p; 
    double M[k][N]; 
    
    srand(time(NULL) + id); // Initialisation de l'aléatoire spécifique à l'entité

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < N; j++) {
            // Remplissage factice de la sous-matrice
            M[i][j] = (double)(rand() % 100); 
        }
    }

    // 2. PRÉPARATION DU MESSAGE (Tableau de 2 entiers aléatoires)
    // Indice 0 : Nombre d'anomalies détectées
    // Indice 1 : Temps de calcul (en ms)
    int msg_envoye[2];
    msg_envoye[0] = rand() % 5;   // Entre 0 et 4 anomalies
    msg_envoye[1] = rand() % 100; // Entre 0 et 99 ms

    printf("Entité %d a terminé. Bilan local : [%d anomalies, %d ms]\n", id, msg_envoye[0], msg_envoye[1]);

    int msg_recu[2] = {0, 0}; 

    MPI_Barrier(MPI_COMM_WORLD); 

    // 3. LA RÉDUCTION COLLECTIVE 
    // Explication des arguments :
    // - msg_envoye : Le tableau de départ
    // - msg_recu   : Le tableau d'arrivée (utile uniquement pour la racine 0)
    // - 2          : La TAILLE du tableau (c'est ça qui indique qu'on traite 2 entiers d'un coup)
    // - MPI_SUM    : L'opération (Addition élément par élément)
    // - 0          : L'entité racine qui centralise le résultat
    
    MPI_Reduce(msg_envoye, msg_recu, 2, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // 4. AFFICHAGE DU RÉSULTAT PAR LA RACINE
    if (id == 0) {
        printf("RAPPORT GLOBAL (Calculé par l'entité 0 via MPI_Reduce)\n");
        printf("-> Total des anomalies dans la matrice : %d\n", msg_recu[0]);
        printf("-> Somme des temps de calcul           : %d ms\n", msg_recu[1]);
    }

    MPI_Finalize();
    return 0;
}*/