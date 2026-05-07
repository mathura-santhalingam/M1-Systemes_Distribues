#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char** argv){
    // question a) graphe complet, MPI_COMM_WORLD

    int n, id_global, msg;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    if(n != 6){
        if (id_global == 0) {
            printf("Erreur, il faut 6 entités dre calculs.\n");
            MPI_Finalize();
            return 0;
        }
    }
    if (id_global == 0) {
        srand(time(NULL)+id_global);
        msg = rand()%100;
    }

    // question b)
    MPI_Comm nv_comm;

    int couleur;
    if(id_global < 3) {
        couleur = 0;
    } else {
        couleur = 1;
    }
    // int couleur = (id_global < 3) ? 0 : 1;
    
    int nv_id = (id_global - id_global % 3) / 3;
    MPI_Comm_split(MPI_COMM_WORLD, couleur, nv_id, nv_comm);

    // question c)
    MPI_Bcast(&msg, 1, MPI_INT, 0, MPI_COMM_WORLD); // pour que 0 diffuse à tout le monde
    // question d)
    MPI_Bcast(&msg, 1, MPI_INT, 0, nv_comm); // pour que 0 diffuse à 1,2 d'un côté et 3 diffuse à 4,5 de l'autre

    if (id_global < 3) {
        MPI_Bcast(&msg, 1, MPI_INT, 0, nv_comm); // pour que SEULEMENT 0 diffuse à 1,2 
    }

    // question e)
    int dims[2] = {3, 2};
    int periodes[2] = {0,1};
    MPI_Cart_create(MPI_COMM_WORLD, 2,dims, periodes, 1, &nv_comm);

    // question f)
    int gauche, droite; // colonne -> axe horizontal
    MPI_Cart_shift(nv_comm, 1, 1, &gauche, &droite);

    MPI_Finalize();
    return 0;
}