#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/*
   Compilation : mpicc -Wall gauss_jordan.c -o gauss_jordan.exe
   Exécution   : mpirun -np 4 ./gauss_jordan.exe
*/

int main(int argc, char** argv) {
    int n, id, i;
    int k = 4;
    int tableau[k];
    int buffer[k]; // Buffer utilisé pour la diffusion, puis pour l'échange final

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    srand(time(NULL) + id);

    // 1. INITIALISATION
    if(id == 0 || id == 1) {
        for (i = 0; i < k; i++) {
            tableau[i] = 0;
        }
    } else {
        for (i = 0; i < k; i++) {
            tableau[i] = rand() % 11;
        }
    }

    // Affichage de départ
    printf("INIT - Ent %d [ ", id);
    for (i = 0; i < k; i++) {
        printf("%d ", tableau[i]);
    }
    printf("]\n");

    // 2. RECHERCHE DU PREMIER NON-NUL (Boucle Bcast)
    int pivot_trouve = 0;  // Booléen : 1 si on a trouvé, 0 sinon
    int id_cible = -1;     // Qui a la ligne avec le pivot ?

    // Chacun à son tour devient la source de la diffusion
    for (int source = 0; source < n; source++) {
        
        // Si c'est mon tour, je copie MON tableau dans le buffer
        if (id == source) {
            for (i = 0; i < k; i++) {
                buffer[i] = tableau[i];
            }
        }

        // Diffusion du buffer
        MPI_Bcast(buffer, k, MPI_INT, source, MPI_COMM_WORLD);

        // Analyse du buffer si on n'a encore rien trouvé
        if (pivot_trouve == 0) {
            for (i = 0; i < k; i++) {
                if (buffer[i] != 0) {
                    pivot_trouve = 1;       // Verrouillage !
                    id_cible = source;      // Mémorisation du propriétaire de la ligne
                    break;
                }
            }
        }
    }

    // 3. L'ÉCHANGE FINAL
    // On échange la ligne entière entre l'entité 0 et l'entité id_cible.
    if (pivot_trouve == 1 && id_cible != 0) {
        
        if (id == 0) {
            // L'entité 0 ENVOIE en premier, puis REÇOIT
            MPI_Send(tableau, k, MPI_INT, id_cible, 0, MPI_COMM_WORLD);
            MPI_Recv(buffer, k, MPI_INT, id_cible, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // On remplace le tableau par ce qu'on a reçu dans le buffer
            for (i = 0; i < k; i++) {
                tableau[i] = buffer[i];
            }
        } 
        else if (id == id_cible) {
            // L'entité cible REÇOIT en premier, puis ENVOIE
            // (Elle reçoit dans le buffer temporel pour ne pas écraser sa propre ligne avant de l'envoyer)
            MPI_Recv(buffer, k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(tableau, k, MPI_INT, 0, 0, MPI_COMM_WORLD);
            
            // On remplace le tableau par ce qu'on a reçu dans le buffer
            for (i = 0; i < k; i++) {
                tableau[i] = buffer[i];
            }
        }
    }
    
    // NB : Si id_cible == 0, l'entité 0 a déjà la bonne ligne, pas besoin de communication réseau !

    // 4. AFFICHAGE FINAL
    MPI_Barrier(MPI_COMM_WORLD);
    printf("FIN  - Ent %d [ ", id);
    for (i = 0; i < k; i++) {
        printf("%d ", tableau[i]);
    }
    printf("]\n");
    
    MPI_Finalize();
    return 0;
}