#include <stdio.h>
#include <stdlib.h>
#include "mpi.h" //chargement de la librairie MPI

/*  mpicc -Wall exo1_2_3.c -o exo1_2_3
    mpirun -np nb_processus ./exo1_2_3
*/

int main(){
    int nb_processus, identifiant;
    double deb, fin, duree;
    //exo1 : printf("Bonjour!");

    // MPI_Init déclare un communicateur, le MPI_COMM_WORLD qui contient les différents processus
    MPI_Init(NULL,NULL); // (NULL,NULL) -> (&argc, &argv)

    deb = MPI_Wtime();

    // Pour obtenir le nb_processus qui est un int définit dans le main :
    MPI_Comm_size(MPI_COMM_WORLD,&nb_processus);

    // Pour obtenir l'identifiant du processus courant, entre 0 et n-1 :
    MPI_Comm_rank(MPI_COMM_WORLD,&identifiant);

    printf("Bonjour, je suis le processus %d parmi %d processsus \n", identifiant, nb_processus);
    
    fin = MPI_Wtime();
    duree = fin - deb;
    printf("La durée est de : %f secondes \n", duree);

    // Pour fermer MPI :
    MPI_Finalize();
    return 0;
}