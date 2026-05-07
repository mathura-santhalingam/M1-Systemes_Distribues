#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/* IMPLÉMENTATION DE :
    Admettons que chaque entité de calcul a un tableau M de k cases.
    ÉTAPE 1 : Les deux premieres entités sont remplies de 0 et les autres ont des valeurs aléatoires.
    ÉTAPE 2 : chaque entité lit son tableau, si elle trouve que des 0 elle ne fait rien.
    On veut trouver, parmi les tableaux, le premier tableau qui a une valeur non nulle.
    Donc le id le plus petit tel que le M de ce id contient une valeur non nulle.
    Ensuite : une fois trouvé cet id, échange sa 1ère valeur non nulle avec la valeur 0 de l’entité 0 à la même position.
    
    Attention : utilise Bcast(buffer, k, MPI_Type, source, MPI_COMM_WORLD);
    Attention : le buffer de reception ne doit pas ecraser directement la valeur dans le tableau car par ex un bcast d'une entité de grand id peut écraser la valeur qu'on cherchait 

*/

/*
   Compilation : mpicc -Wall gauss_jordan_easy_v1.c -o gauss_jordan_easy_v1.exe
   Exécution   : mpirun -np 7 ./gauss_jordan_easy_v1.exe
*/


int main(int argc, char** argv) {
    int n, id, i;
    int k = 4;
    int tableau[k];
    int buffer[k]; //buffer pour protéger notre tableau

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    srand(time(NULL) + id);

    // Initialisation
    if(id == 0 || id == 1) {
        for (i = 0; i < k; i++) {
            tableau[i] = 0;
        }
    } else {
        for (i = 0; i < k; i++) {
            tableau[i] = rand() % 11;
        }
    }

    // Affichage de départ pour vérifier
    printf("INIT - Ent %d [ ", id);
    for (i = 0; i < k; i++) {
        printf("%d ", tableau[i]);
    }
    printf("]\n");

    // RECHERCHE DU PREMIER NON-NUL (boucle Bcast)
    int pivot_trouve = 0;  // Booléen : 1 si on a trouvé, 0 sinon
    int id_cible = -1;     // Qui a la valeur ?
    int index_cible = -1;  // A quelle position ?
    int valeur_cible = 0;  // Quelle est la valeur ?

    // Chacun à son tour devient la source de la diffusion
    for (int source = 0; source < n; source++) {
        
        // Si c'est mon tour, je copie MON tableau dans le buffer pour l'envoyer
        if (id == source) {
            for (i = 0; i < k; i++) {
                buffer[i] = tableau[i];
            }
        }

        // L'entité source envoie son buffer à tout le monde
        // Tout le monde (y compris la source) reçoit dans buffer
        MPI_Bcast(buffer, k, MPI_INT, source, MPI_COMM_WORLD);

        // Si on n'a PAS ENCORE trouvé de pivot lors des tours précédents
        if (pivot_trouve == 0) {
            // On analyse le buffer qu'on vient de recevoir
            for (i = 0; i < k; i++) {
                if (buffer[i] != 0) {
                    pivot_trouve = 1;       // Verrouillage ! On ne cherchera plus
                    id_cible = source;      // On mémorise le propriétaire
                    index_cible = i;        // On mémorise la case
                    valeur_cible = buffer[i]; // On mémorise le nombre
                    break; // On sort de la boucle d'analyse du buffer
                }
            }
        }
    }

    // L'ÉCHANGE FINAL
    // ici TOUT LE MONDE sait exactement qui a la valeur et où elle est,
    // car tout le monde a analysé les mêmes buffers dans le même ordre !
    
    if (pivot_trouve == 1) {
        if (id == 0) {
            // L'entité 0 remplace son '0' par la valeur trouvée
            tableau[index_cible] = valeur_cible;
        }
        
        if (id == id_cible && id != 0) {
            // L'entité qui possédait la valeur la remplace par '0'
            tableau[index_cible] = 0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("FIN  - Ent %d [ ", id);
    for (i = 0; i < k; i++) {
        printf("%d ", tableau[i]);
    }
    printf("]\n");
    
    MPI_Finalize();
    return 0;
}