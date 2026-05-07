#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// IMPLÉMENTATION : diffuser un msg à partir de l'entité 1 vers toutes les autres entités d'identifiant impair et uniquement celles-là (Bcast)

int main(int argc, char** argv){
    int msg;
    int n, id_global;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    MPI_Comm nv_comm;

    int nv_id = (id_global -(id_global % 2) ) /2;

    int couleur = (id_global % 2 == 1) ? 0 : MPI_UNDEFINED;

    // Création du communicateur
    MPI_Comm_split(MPI_COMM_WORLD, couleur, nv_id, &nv_comm);

    // Sécurisation : Seuls les impairs (ceux qui ont un communicateur valide) participent
    if (nv_comm != MPI_COMM_NULL) {
            if (nv_id == 0) {
            msg = 42; 
        }
        // Diffusion avec la racine locale 0 (qui correspond bien au global 1)
        MPI_Bcast(&msg, 1, MPI_INT, 0, nv_comm);
    }
    if (nv_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&nv_comm);
    }
    MPI_Finalize();
    return 0;
}