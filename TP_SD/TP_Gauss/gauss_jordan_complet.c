#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

/*
   Compilation : mpicc -Wall gauss_jordan_complet.c -o gauss_jordan_complet.exe
   Exécution   : mpirun -np 4 ./gauss_jordan_complet.exe
*/

int main(int argc, char** argv) {
    int n, id, i, j;
    
    // Paramètres de la matrice
    int k = 3;  // Nombre de lignes de la matrice par entité
    int nc = 4; // Nombre de colonnes de la matrice
    
    int M[k][nc];
    int buffer[k][nc]; // Buffer 2D pour la diffusion

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    srand(time(NULL) + id);

    // 1. INITIALISATION DE LA MATRICE 2D
    for (i = 0; i < k; i++) {
        for (j = 0; j < nc; j++) {
            // On force des zéros sur la toute première ligne de l'entité 0 pour être sûr 
            // qu'elle doive chercher un pivot ailleurs !
            if (id == 0 && i == 0) {
                M[i][j] = 0;
            } else {
                M[i][j] = rand() % 10;
            }
        }
    }

    // 2. PARAMÈTRES DU PIVOT ACTUEL
    // Imaginons que nous traitons la 1ère équation du système (ligne globale l = 1)
    int l = 1; 
    int col_pivot = l - 1; // Le pivot se cherche dans la colonne 0
    
    // L'entité qui DOIT posséder cette ligne globale
    int id_dest = (l - 1) / k; 
    // L'index local de cette ligne dans l'entité de destination
    int local_dest = (l - 1) % k; 

    // 3. RECHERCHE DU PIVOT (Boucle Bcast avec Buffer)
    int pivot_trouve = 0;
    int id_cible = -1;     // Quelle entité a le pivot ?
    int index_cible = -1;  // À quelle ligne locale ?

    for (int source = 0; source < n; source++) {
        
        // La source copie sa matrice dans le buffer
        if (id == source) {
            for (i = 0; i < k; i++) {
                for (j = 0; j < nc; j++) {
                    buffer[i][j] = M[i][j];
                }
            }
        }

        // Diffusion du buffer complet (k * nc éléments)
        // L'adresse &buffer[0][0] garantit que le C lit bien le tableau 2D comme un bloc continu
        MPI_Bcast(&buffer[0][0], k * nc, MPI_INT, source, MPI_COMM_WORLD);

        // Si on n'a pas encore trouvé le pivot, on analyse le buffer reçu
        if (pivot_trouve == 0) {
            // On ne cherche que dans les entités qui sont à la suite de id_dest (inutile de chercher au-dessus)
            if (source >= id_dest) {
                
                // Si on est dans id_dest, on cherche à partir de 'local_dest'. Sinon, on cherche dès la ligne 0.
                int ligne_depart = (source == id_dest) ? local_dest : 0;
                
                for (i = ligne_depart; i < k; i++) {
                    if (buffer[i][col_pivot] != 0) { // On a trouvé une valeur non-nulle dans la bonne colonne !
                        pivot_trouve = 1;
                        id_cible = source;
                        index_cible = i;
                        break;
                    }
                }
            }
        }
    }

    // 4. L'ÉCHANGE FINAL (Ligne entière de 'nc' éléments)
    if (pivot_trouve == 1) {
        
        // CAS A : Le pivot est dans une AUTRE entité -> Échange réseau sécurisé (Sans Sendrecv)
        if (id_cible != id_dest) {
            int temp_ligne[nc]; // Petit buffer temporaire juste pour réceptionner la ligne
            
            if (id == id_dest) {
                // id_dest ENVOIE d'abord, puis REÇOIT
                MPI_Send(&M[local_dest][0], nc, MPI_INT, id_cible, 0, MPI_COMM_WORLD);
                MPI_Recv(temp_ligne, nc, MPI_INT, id_cible, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                for (j = 0; j < nc; j++) M[local_dest][j] = temp_ligne[j];
                printf("=> Entité %d a récupéré la ligne pivot depuis l'entité %d via le réseau.\n", id, id_cible);
            } 
            else if (id == id_cible) {
                // id_cible REÇOIT d'abord, puis ENVOIE
                MPI_Recv(temp_ligne, nc, MPI_INT, id_dest, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&M[index_cible][0], nc, MPI_INT, id_dest, 0, MPI_COMM_WORLD);
                
                for (j = 0; j < nc; j++) M[index_cible][j] = temp_ligne[j];
            }
        } 
        // CAS B : Le pivot est DANS LA MÊME entité, mais sur une ligne plus basse -> Échange local en mémoire (Swap)
        else if (id_cible == id_dest && index_cible != local_dest) {
            if (id == id_dest) {
                int temp_val;
                for (j = 0; j < nc; j++) {
                    temp_val = M[local_dest][j];
                    M[local_dest][j] = M[index_cible][j];
                    M[index_cible][j] = temp_val;
                }
                printf("=> Entité %d a fait un swap local (ligne %d <-> ligne %d).\n", id, local_dest, index_cible);
            }
        }
        // CAS C : Le pivot était déjà parfaitement à sa place ! Rien à faire.
    }

    MPI_Finalize();
    return 0;
}