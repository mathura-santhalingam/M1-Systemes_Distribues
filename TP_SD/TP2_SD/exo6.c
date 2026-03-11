#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

/*  mpicc -Wall exo6.c -o exo6
    mpirun -np 8 ./exo6
*/

// Définition de d avec une directive #define
#define d 2 

int main(int argc, char** argv) {
    int n, id, i, j;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Initialisation du hasard (différent pour chaque processus)
    srand(time(NULL) + id);

    // Génération de d nombres distincts v1 ... vd
    int v[d];
    for (i = 0; i < d; i++) {
        int val, deja_choisi;
        do {
            deja_choisi = 0;
            val = rand() % n; 
            
            // On vérifie qu'on ne se choisit pas soi-même ( {0..i-1} U {i+1..n-1} )
            if (val == id) deja_choisi = 1; 
            
            // On vérifie que le nombre est bien distinct des précédents
            for (j = 0; j < i; j++) {
                if (v[j] == val) deja_choisi = 1; 
            }
        } while (deja_choisi); // On recommence si la valeur n'est pas bonne
        
        v[i] = val; // On stocke le choix validé
    }

    // Tableau voisin
    int voisin[n]; // Taille maximale possible : n
    int nb_voisins = 0;

    // Remplissage du tableau voisin avec d'abord les d entités que j'ai personnellement choisies
    for (i = 0; i < d; i++) {
        voisin[nb_voisins] = v[i];
        nb_voisins++;
    }

    // Envoi d'un message aux entités choisies
    for (i = 0; i < d; i++) {
        // J'envoie mon propre identifiant ('id') à l'entité v[i]
        MPI_Send(&id, 1, MPI_INT, v[i], 0, MPI_COMM_WORLD);
    }

    /* --- ASTUCE TECHNIQUE OBLIGATOIRE --- 
       Pour respecter la règle "peut en avoir plus que d", il faut réceptionner
       les messages des autres. Mais comme on ne sait pas combien de processus 
       nous ont choisi au hasard, on fait un "Allreduce" pour le calculer 
       et éviter que le programme ne plante (deadlock).
    */
    int mes_choix[n], qui_me_veut[n];
    for (i = 0; i < n; i++) mes_choix[i] = 0;
    for (i = 0; i < d; i++) mes_choix[v[i]] = 1;
    MPI_Allreduce(mes_choix, qui_me_veut, n, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    int nb_receptions = qui_me_veut[id];


    // --- SUITE DU POINT 3 : Recevoir et stocker les autres voisins ---
    for (i = 0; i < nb_receptions; i++) {
        int expediteur;
        MPI_Status status;
        MPI_Recv(&expediteur, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        
        // On vérifie qu'il n'est pas déjà dans le tableau voisin (cas où on s'est choisis mutuellement)
        int existe = 0;
        for (j = 0; j < nb_voisins; j++) {
            if (voisin[j] == expediteur) existe = 1;
        }
        
        // S'il n'y est pas, on l'ajoute !
        if (!existe) {
            voisin[nb_voisins] = expediteur;
            nb_voisins++;
        }
    }

    // --- AFFICHAGE DE VÉRIFICATION ---
    printf("Processus %d a %d voisins : ", id, nb_voisins);
    for (i = 0; i < nb_voisins; i++) printf("%d ", voisin[i]);
    printf("\n");

    MPI_Finalize();
    return 0;
}