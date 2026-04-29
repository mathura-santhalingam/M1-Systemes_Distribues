#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

/*  mpicc -Wall exo7.c -o exo7.exe
    mpirun -np 2 ./exo7.exe
*/

int main(int argc, char** argv) {
    int n, id;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id == 0) {
        printf("Un groupe isolé doit donc contenir au minimum l'entité elle-même + ses d voisins. \nLa taille minimale d'un groupe isolé est donc de d + 1 entités.\nSi le graphe est coupé en deux groupes isolés, il faut au moins d + 1 entités dans le groupe A, et d + 1 entités dans le groupe B.\nLe nombre total d'entités n doit donc vérifier :\nn >= (d + 1) + (d + 1)\nn >= 2d + 2\n ");
        printf("La seule valeur en dessous de laquelle le graphe est garanti d'être non-connexe est d < 1 (c'est-à-dire d = 0, où personne ne choisit de voisin).");
    }
    MPI_Finalize();
    return 0;
}