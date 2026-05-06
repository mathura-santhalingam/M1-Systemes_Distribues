#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// IMPLÉMENTATION : Réduction (opération max)

/*
   Compilation : mpicc -Wall exo2.c -o exo2.exe
   Exécution   : mpirun -np 8 ./exo2.exe
*/

int main(int argc, char** argv) {
    int id, nb_processus;
    int msg_recu;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus);

    srand(time(NULL) + id);
    int a_i = rand() % 100;
    printf("Entité %d génère la valeur %d\n", id, a_i);

    int parent = (id - 1) / 2;
    int fils_gauche = 2 * id + 1;
    int fils_droit = 2 * id + 2;

    // nombre de fils (c = |fils(u)|)
    int c = 0;
    if (fils_gauche < nb_processus) c++;
    if (fils_droit < nb_processus) c++;

    if (c == 0 && id != 0) {
        // si feuille (0 enfant)
        MPI_Send(&a_i, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
    } else {
        // si noeud interne (ou la racine)
        while (c > 0) {            
            MPI_Recv(&msg_recu, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (msg_recu > a_i) {
                a_i = msg_recu;
            }
            c--;
        }
        // si on n'est pas la racine on remonte le max calculé au père
        if (id != 0) {
            MPI_Send(&a_i, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
        }
    }
    if (id == 0) {
        printf("\nMaximum global = %d\n", a_i);
    }
    MPI_Finalize();
    return 0;
}