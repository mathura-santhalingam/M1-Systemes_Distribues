#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

// IMPLÉMENTATION : Ping-Pong sur N tours

/*  mpicc -Wall exo5_bis.c -o exo5_bis
    mpirun -np nb_processus ./exo5_bis
*/

// Attention, MPI synchronise les messages, pas les prints
// ping-pong
int main(int argc, char** argv){

    int nb_processus, identifiant;
    // int msg_recu; ATTENTION NON CAR SINON PLUS EFFET PING PONG, COMME SI ON AVAIT DEUX BALLES
    int N;
    int k = 100;
    int m = 200;
    int msg;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD, &identifiant);
    MPI_Status status;

    // Vérification du nombre de processus :
    if (nb_processus != 2){
        if(identifiant == 0) { // pour n'afficher qu'une fois le message d'erreur
            printf("Erreur : il faut exactement 2 processus.");
        }
        MPI_Finalize();
        return 1;
    }

    // Vérification de la présence de l'argument N (nombre de tours) :
    if (argc != 2) {
        if(identifiant == 0){
            printf("Erreur : Veuillez préciser le nombre de tours N : mpirun -np 2 ./exo6 <N>\n");
        }
        MPI_Finalize();
        return 1;
    }

    // On a bien en argument N, donc on le met dans la variable n :
    N = atoi(argv[1]); // Convertit l'argument texte en entier

    // Ping-pong :
    for (int i = 0; i < N; i++) {
        if(identifiant == 0){
            printf("[tour %d] : Je suis %d et j'envoie la valeur %d à %d.\n", i+1, identifiant, m, (identifiant+1)%2);
            MPI_Send(&m,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);

            MPI_Recv(&msg,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
            printf("[tour %d] : Je suis %d et j'ai reçu %d de %d.\n", i+1, identifiant, msg, (identifiant+1)%2);

        } else {
            MPI_Recv(&msg,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
            printf("[tour %d] : Je suis %d et j'ai reçu %d de %d.\n", i+1, identifiant, msg, (identifiant+1)%2);

            printf("[tour %d] : Je suis %d et j'envoie la valeur %d à %d.\n", i+1, identifiant, k, (identifiant+1)%2);
            MPI_Send(&k,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
    return 0;
}