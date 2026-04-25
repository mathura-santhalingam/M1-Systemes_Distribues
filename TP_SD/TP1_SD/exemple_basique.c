#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

/* IMPLÉMENTATION DE l'EXEMPLE DE BASE : Algorithme du Ping-Pong (Échange itératif et incrémental)
Les rôles changent à chaque tour de boucle grâce à la condition k % 2 == id.
Quand k=0 (pair), le processus 0 envoie et le 1 reçoit.
Quand k=1 (impair), le processus 1 envoie et le 0 reçoit.
Effet ping-pong. */

/*  mpicc -Wall exemple_basique.c -o exemple_basique
    mpirun -np 2 ./exemple_basique

    MPI_Send(buffer,k,MPI_Type,destination_id,balise,MPI_COMM_WORLD);
    MPI_Recv(buffer,k,MPI_Type,origine_id,balise,MPI_COMM_WORLD,&status);
*/

int main(int argc, char** argv) {
    int n, id;
    int k = 0;
    int e = 0;
    int r = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Status status;

    while (k < 4) {
        // au lieu de faire "if(id == 0) else"
        if(k % 2 == id){
            MPI_Send(&k, 1, MPI_INT, (id+1)%2, 0, MPI_COMM_WORLD);
            e++;
        } else {
            MPI_Recv(&k, 1, MPI_INT, (id+1)%2, 0, MPI_COMM_WORLD, &status);
            r++;
        }
        k++;
    }
    printf("Ent %d : e = %d et r = %d\n", id, e, r);
    MPI_Finalize();
    return 0;
}