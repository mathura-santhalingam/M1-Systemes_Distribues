#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

/*  mpicc -Wall exo8.c -o exo8
    mpirun -np nb_processus ./exo8
*/

int main(){
    int nb_processus, identifiant;
    int msg_recu;
    int k = 100;
    int m = 200;
    int n = 4;
    int i;

    MPI_Init(NULL,NULL); // déclare un communicateur : MPI_COMM_WORLD
    MPI_Comm_size(MPI_COMM_WORLD,&nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD,&identifiant);
    MPI_Status status;

    if (nb_processus != n){
        printf("Erreur : il faut exactement n processus.");
        MPI_Finalize();
        exit(1);
    }
    if(identifiant == 0){
        //printf("Je suis %d et j'envoie la valeur %d à %d.\n", identifiant, m, (identifiant+1)%2);
        MPI_Send(&m,1,MPI_INT,((identifiant+1)%nb_processus),0,MPI_COMM_WORLD);
        MPI_Recv(&msg_recu,1,MPI_INT,((identifiant-1+ nb_processus)%nb_processus),0,MPI_COMM_WORLD,&status);
        //printf("Je suis %d et j'ai reçu %d de %d.\n", identifiant, msg_recu, (identifiant+1)%2);
    } else {
        MPI_Recv(&msg_recu,1,MPI_INT,((identifiant-1+ nb_processus)%nb_processus),0,MPI_COMM_WORLD,&status);
        //printf("Je suis %d et j'ai reçu %d de %d.\n", identifiant, msg_recu, (identifiant+1)%2);
        //printf("Je suis %d et j'envoie la valeur %d à %d.\n", identifiant, k, (identifiant+1)%2);
        MPI_Send(&k,1,MPI_INT,((identifiant+1)%nb_processus),0,MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}