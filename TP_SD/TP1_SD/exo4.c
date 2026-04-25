#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

/* IMPLÉMENTATION D'UN ÉCHANGE ENTRE DEUX PROCESSUS : Échange croisé des identifiants (Communication asymétrique)
Les rôles sont codés en dur selon l'identifiant.
Le processus 0 envoie puis reçoit. Le processus 1 reçoit puis envoie.
C'est parfait pour UN SEUL échange (lourd pour aller-retour) */

/*  mpicc -Wall exo4.c -o exo4
    mpirun -np nb_processus ./exo4

    MPI_Send(buffer,k,MPI_Type,destination_id,balise,MPI_COMM_WORLD);
    MPI_Recv(buffer,k,MPI_Type,origine_id,balise,MPI_COMM_WORLD,&status); avec status de type MPI_Status qui donne détails sur msg_recu
*/

// Échange entre de 

int main(){
    int nb_processus, identifiant;
    int msg_recu;

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD,&identifiant);
    MPI_Status status;

    if (nb_processus != 2){
        if(identifiant == 0) { // pour n'afficher qu'une fois le message d'erreur
            printf("Erreur : il faut exactement 2 processus.");
        }        
        MPI_Finalize();
        exit(1);
    }

    // On a vérifié qu'il y avait bien 2 processus donc on passe à la suite :
    if(identifiant == 0){
        MPI_Send(&identifiant,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);

        MPI_Recv(&msg_recu,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
        printf("Je suis %d et j'ai reçu l'identifiant de %d.\n", identifiant, (identifiant+1)%2);
    } else {
        MPI_Recv(&msg_recu,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD,&status);
        printf("Je suis %d et j'ai reçu l'identifiant de %d.\n", identifiant, (identifiant+1)%2);
        
        MPI_Send(&identifiant,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}