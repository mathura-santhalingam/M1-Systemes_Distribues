#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

/*  mpicc -Wall exo8.c -o exo8
    mpirun -np nb_processus ./exo8
*/

int main(int argc, char** argv) {
    int n, id;
    int M = 100;
    int ma_valeur;
    int min_max[2]; // min_max[0] stocke le minimum, min_max[1] stocke le maximum
    /* CAS 2 MSG AVEC BALISE DISCRIMINANTE :
    int min_global;
    int max_global */

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Status status;

    // Pour que le hasard soit différent sur chaque processus
    srand(time(NULL) + id); 
    ma_valeur = rand() % (M + 1);
    printf("Processus %d a généré la valeur %d\n", id, ma_valeur);

    if (id == 0) {
        min_max[0] = ma_valeur;
        min_max[1] = ma_valeur;
        // Envoi d'un bloc de 2 entiers
        MPI_Send(min_max, 2, MPI_INT, 1, 0, MPI_COMM_WORLD);
        /* CAS 2 MSG AVEC BALISE DISCRIMINANTE : (L'avant-dernier argument est la balise)
        MPI_Send(&min_global, 1, MPI_INT, id + 1, 1, MPI_COMM_WORLD); // Balise 1 = MIN
        MPI_Send(&max_global, 1, MPI_INT, id + 1, 2, MPI_COMM_WORLD); // Balise 2 = MAX
        */
    } 
    else {
        // Réception du bloc de 2 entiers
        MPI_Recv(min_max, 2, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
        /* CAS 2 MSG AVEC BALISE DISCRIMINANTE : (L'avant-dernier argument est la balise)
        // On force la réception avec la balise spécifique pour être sûr de qui est quoi
        MPI_Recv(&min_global, 1, MPI_INT, id - 1, 1, MPI_COMM_WORLD, &status); 
        MPI_Recv(&max_global, 1, MPI_INT, id - 1, 2, MPI_COMM_WORLD, &status);
        */
        // Mise à jour si nécessaire
        if (ma_valeur < min_max[0]) min_max[0] = ma_valeur;
        if (ma_valeur > min_max[1]) min_max[1] = ma_valeur;
        
        // Si je ne suis pas le dernier, je transmets au suivant
        if (id < n - 1) {
            MPI_Send(min_max, 2, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
            /* CAS 2 MSG AVEC BALISE DISCRIMINANTE : (L'avant-dernier argument est la balise)
            MPI_Send(&min_global, 1, MPI_INT, id + 1, 1, MPI_COMM_WORLD); // Balise 1 = MIN
            MPI_Send(&max_global, 1, MPI_INT, id + 1, 2, MPI_COMM_WORLD); // Balise 2 = MAX
            */
        } 
        // Si je suis le dernier, j'affiche le résultat final
        else {
            printf("Le minimum global est : %d\n", min_max[0]);
            printf("Le maximum global est : %d\n", min_max[1]);
        }
    }

    /* CAS 3 : 2 MSG NON DISCRIMINÉS PAR BALISE = 2 MSG AVEC MÊME BALISE :
    MPI_Send(&min_global, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
    MPI_Send(&max_global, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);

    et
    MPI_Recv(&min_global, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&max_global, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
    */

    MPI_Finalize();
    return 0;
}