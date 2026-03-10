#include <stdio.h>
#include <stdlib.h>
#include "mpi.h" //chargement de la librairie MPI

/*  mpicc -Wall exo5.c -o exo5
    mpirun -np nb_processus ./exo5
*/

int main(){

    int nb_processus, identifiant, msg_recu;
    int k = 100;
    int m = 200;

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD,&identifiant);

    MPI_Status status;

    if (nb_processus != 2){
        printf("Erreur : il faut exactement 2 processus.");
        MPI_Finalize();
        exit(1);
    }

    if(identifiant == 0){
        MPI_Send(&m,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
        printf("Je suis %d et j'envoie la valeur %d à %d.\n", identifiant, m, (identifiant+1)%2);

        MPI_Recv(&msg_recu,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
        printf("Je suis %d et j'ai reçu %d de %d.\n", identifiant, msg_recu, (identifiant+1)%2);
    } else {
        MPI_Recv(&msg_recu,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
        printf("Je suis %d et j'ai reçu %d de %d.\n", identifiant, msg_recu, (identifiant+1)%2);

        MPI_Send(&k,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
        printf("Je suis %d et j'envoie la valeur %d à %d.\n", identifiant, k, (identifiant+1)%2);
    }

    MPI_Finalize();
    return 0;
}