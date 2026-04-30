#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main()
{
    int size, rank;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int message, *buffer_r;
    
    if(rank!=0) message=rank;
    
    buffer_r=(int *)malloc((size-1)*sizeof(int));
    MPI_Gather(&message, 1, MPI_INT, buffer_r, 1, MPI_INT,0, MPI_COMM_WORLD);
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}

/* MPI_Gather est strictement ordonné par le rang (l'identifiant) des processus.

*   La fonction MPI_Gather place mathématiquement les données dans le buffer_r en utilisant le rang comme index,
    peu importe qui a envoyé son message en premier sur le réseau.
*   Les données du Processus 0 vont au tout début du buffer.
*   Les données du Processus 1 se placent juste après.
*   Les données du Processus i se placent avec un décalage exact de i * k_r (où k_r est la taille du message de chaque entité).

taille n-1 => décalage pcq pas racine ?


EXEMPLE MATRICE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char** argv) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // 1. PRÉPARATION DU MESSAGE LOCAL
    // Chaque entité construit son tableau de 2 entiers aléatoires
    
    int k_e = 2; // Taille du message envoyé par chaque entité
    int msg_envoye[k_e]; 
    
    srand(time(NULL) + rank); // Graine unique pour avoir des nombres différents
    
    msg_envoye[0] = rand() % 50;  // Valeur 1 (ex: température)
    msg_envoye[1] = rand() % 100; // Valeur 2 (ex: taux d'humidité)

    // Affichage local pour vérifier ce que chacun va envoyer
    printf("Processus %d a généré : [%d, %d]\n", rank, msg_envoye[0], msg_envoye[1]);

    // Barrière purement esthétique pour l'affichage console
    MPI_Barrier(MPI_COMM_WORLD); 

    // 2. PRÉPARATION DE LA MATRICE DE RÉCEPTION SUR LA RACINE    
    int *matrice_reception = NULL;
    
    if (rank == 0) {
        // LA FAMEUSE CORRECTION DU COURS :
        // On a 'size' processus en tout (racine incluse !).
        // Chacun envoie 'k_e' (soit 2) entiers.
        // Taille totale nécessaire = size * 2
        matrice_reception = (int*)malloc(size * k_e * sizeof(int));
    }

    // 3. LA CONCENTRATION (GATHER)
    
    // Règle d'or de MPI_Gather : le paramètre 'count' (ici k_e) indique
    // le nombre d'éléments envoyés PAR processus, et non le total !
    MPI_Gather(
        msg_envoye, k_e, MPI_INT,        // Ce que j'envoie (2 entiers)
        matrice_reception, k_e, MPI_INT, // Ce que la racine reçoit (2 entiers PAR processus)
        0, MPI_COMM_WORLD                // Le rang de la racine (0) et le communicateur
    );

    // 4. EXPLOITATION DE LA MATRICE GLOBALE PAR LA RACINE
    
    if (rank == 0) {
        printf("\n--- RÉSULTAT SUR LA RACINE (Construction de la matrice) ---\n");
        
        // La racine parcourt le grand tableau reconstruit
        for (int i = 0; i < size; i++) {
            // Pour lire les données du processus 'i', on applique le décalage : i * k_e
            int index = i * k_e; 
            printf("Ligne du processus %d : [%d, %d]\n", 
                   i, matrice_reception[index], matrice_reception[index + 1]);
        }
        
        // On n'oublie pas de libérer la mémoire allouée
        free(matrice_reception);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
*/