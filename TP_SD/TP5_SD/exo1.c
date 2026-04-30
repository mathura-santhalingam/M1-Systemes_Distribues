#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*
   Compilation : mpicc -Wall exo1.c -o exo1.exe
   Exécution   : mpirun -np 8 ./exo1.exe
*/

int main(int argc, char ** argv) {
    int id_global, nb_processus_global;
    int buffer = -1;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus_global);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    // Vérification de la présence de l'argument nc :
    if (argc != 2) {
        if(id_global == 0){
            printf("Erreur : Veuillez préciser le nombre de communicateurs nc : mpirun -np 2 ./exo6 <nc>\n");
        }
        MPI_Finalize();
        return 1;
    }

    // On a bien en argument N, donc on le met dans la variable n :
    int nc = atoi(argv[1]); // Convertit l'argument texte en entier


    // Initialisation des nc nouveaux communicateurs
    MPI_Comm comm_nc;

    int couleur = id_global % nc;
    int nv_id = (id_global - id_global % nc) / nc;

    MPI_Comm_split(MPI_COMM_WORLD, couleur, nv_id, &comm_nc);    

    int id_local;
    MPI_Comm_rank(comm_nc, &id_local);
    // Diffusion
    if(id_local == 0) {
        buffer = id_global % nc;
    }

    MPI_Bcast(&buffer, 1, MPI_INT, 0, comm_nc);

    printf("Je suis le global %d (local %d) et j'ai reçu via Bcast depuis 0 :%d\n", id_global, id_local, buffer);
    
    MPI_Comm_free(&comm_nc);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
