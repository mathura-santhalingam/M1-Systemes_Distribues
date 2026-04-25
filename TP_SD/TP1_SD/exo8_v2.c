#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

// IMPLÉMENTATION Calcul du Min/Max par envois séparés via discrimination par balises

/*  mpicc -Wall exo8_v2.c -o exo8_v2.exe
    mpirun -np nb_processus ./exo8_v2.exe
*/

int main(int argc, char** argv) {
    int n, id;
    int M = 100;
    int ma_valeur;
    int min;
    int max;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Status status;

    // Pour que le hasard soit différent sur chaque processus
    srand(time(NULL) + id); 
    ma_valeur = rand() % (M + 1);
    printf("Processus %d a généré la valeur %d\n", id, ma_valeur);

    if (id == 0) {
        min = ma_valeur;
        max = ma_valeur;
        // Envoi de 2 messages de balises différentes
        MPI_Send(&min, 1, MPI_INT, 1, 0, MPI_COMM_WORLD); //min de  balise 0
        MPI_Send(&max, 1, MPI_INT, 1, 1, MPI_COMM_WORLD); //max de  balise 1
    } 
    else {
        // Réception du bloc de 2 entiers
        MPI_Recv(&min, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&max, 1, MPI_INT, id - 1, 1, MPI_COMM_WORLD, &status);
        // Mise à jour si nécessaire
        if (ma_valeur < min) min = ma_valeur;
        if (ma_valeur > max) max = ma_valeur;
        
        // Si je ne suis pas le dernier, je transmets au suivant
        if (id < n - 1) {
            MPI_Send(&min, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
            MPI_Send(&max, 1, MPI_INT, id + 1, 1, MPI_COMM_WORLD);
        } 
        // Si je suis le dernier, j'affiche le résultat final
        else {
            printf("Le minimum global est : %d\n", min);
            printf("Le maximum global est : %d\n", max);
        }
    }
    MPI_Finalize();
    return 0;
}