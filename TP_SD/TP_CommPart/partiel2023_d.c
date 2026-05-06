#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define m 40

/*  IMPLÉMENTATION : Programmez un code qui fait les choses suivantes
*   * chaque entité de calcul fait au hasard un entier k entre 0 et 99
*   * chaque entité de calcul envoie son entier k à ses voisins dans le communicateur
*   * chaque entité de calcul reçoit le maximum et l'affecte à son entier k
*   * on recommence m fois
*/

/*
   Compilation : mpicc -Wall partiel2023_d.c -o partiel2023_d.exe
   Exécution   : mpirun -np 8 ./partiel2023_d.exe
*/


int main(int argc, char** argv){
    int n, id_global, msg_g, msg_d;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);
    MPI_Status status;

    srand(time(NULL) + id_global);
    int k = rand()%100;

    MPI_Comm nv_comm;
    int dims[1]={n};
    int periode[1]={0};
    MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periode, 1, &nv_comm);

    int gauche, droit;
    MPI_Cart_shift(nv_comm, 0, 1, &gauche, &droit);

    int coord;
    MPI_Cart_coords(nv_comm, id_global, 1, &coord);

    for(int i = 0; i < m; i++) {
        if (coord % 2 == 0){
            MPI_Send(&k, 1, MPI_INT, droit, 0, nv_comm);
            MPI_Recv(&msg_d, 1, MPI_INT, droit, 0, nv_comm, &status);

            MPI_Send(&k, 1, MPI_INT, gauche, 0, nv_comm);
            MPI_Recv(&msg_g, 1, MPI_INT, gauche, 0, nv_comm, &status);
        } else {
            MPI_Recv(&msg_g, 1, MPI_INT, gauche, 0, nv_comm, &status);
            MPI_Send(&k, 1, MPI_INT, gauche, 0, nv_comm);

            MPI_Recv(&msg_d, 1, MPI_INT, droit, 0, nv_comm, &status);    
            MPI_Send(&k, 1, MPI_INT, droit, 0, nv_comm);
        }
        if (gauche != MPI_PROC_NULL && msg_g > k) {
            k = msg_g;
        }
        if (droit != MPI_PROC_NULL && msg_d > k) {
            k = msg_d;
        }
        MPI_Barrier(nv_comm);
    }
    printf("Fin : Entité %d a le k final = %d\n", id_global, k);

    MPI_Barrier(nv_comm);
    if(id_global == 0) printf("Si m est très grand, k sera exactement le même pour toutes les entités,\nil sera égal au maximum global car à chaque itération de m,\nk se propage d'un cran en plu vers la gauche et la droite\n");
    MPI_Barrier(nv_comm);
    
    MPI_Comm_free(&nv_comm);
    MPI_Finalize();
    return 0;
}