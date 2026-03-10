#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

/*  mpicc -Wall exo6.c -o exo6
    mpirun -np nb_processus ./exo6
*/

// Attention, MPI synchronise les messages, pas les prints

int main(int argc, char** argv){

    int nb_processus, identifiant;
    int val_a_env, msg_recu;
    int n, i;

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
    n = atoi(argv[1]); // Convertit l'argument texte en entier

    // L'un des processus envoie 100, l'autre envoie 200
    if (identifiant == 0) {
        val_a_env = 100;
    } else {
        val_a_env = 200;
    }

    // Ping-pong :
    for (i = 0; i < n; i++) {
        if(identifiant == 0){
            printf("[tour %d] : Je suis %d et j'envoie la valeur %d à %d.\n", i+1, identifiant, val_a_env, (identifiant+1)%2);
            MPI_Send(&val_a_env,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);

            MPI_Recv(&msg_recu,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
            printf("[tour %d] : Je suis %d et j'ai reçu %d de %d.\n", i+1, identifiant, msg_recu, (identifiant+1)%2);
        } else {
            MPI_Recv(&msg_recu,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
            printf("[tour %d] : Je suis %d et j'ai reçu %d de %d.\n", i+1, identifiant, msg_recu, (identifiant+1)%2);

            printf("[tour %d] : Je suis %d et j'envoie la valeur %d à %d.\n", i+1, identifiant, val_a_env, (identifiant+1)%2);
            MPI_Send(&val_a_env,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
    return 0;
}