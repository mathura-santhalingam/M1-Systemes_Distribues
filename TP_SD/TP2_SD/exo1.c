#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

// IMPLÉMENTATION DE L'ALGO CASTt(r)

/*  mpicc -Wall exo1.c -o exo1
    mpirun -np 4 ./exo1
*/

int main(int argc, char** argv) {
    int n, id;
    int msg_recu[2];
    int val_a_env[2];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n < 2) {
        if (id == 0) {
            printf("Erreur : Il faut au moins 2 processus.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (id == 0) {
    // Création des valeurs aléatoires générées (par la racine) à envoyer
        srand(time(NULL));
        val_a_env[0] = rand() % 100;
        val_a_env[1] = rand() % 100;
        printf("Processus 0 a généré le message : [%d, %d]\n", val_a_env[0], val_a_env[1]);

        // La racine envoie à ses enfants
        int fils_gauche = 2 * id + 1;
        int fils_droit = 2 * id + 2;

        if (fils_gauche < n) {
            MPI_Send(val_a_env, 2, MPI_INT, fils_gauche, 0, MPI_COMM_WORLD);
        }
        if (fils_droit < n) {
            MPI_Send(val_a_env, 2, MPI_INT, fils_droit, 0, MPI_COMM_WORLD);
        }
    } 
    // Relais
    else {
        // Reception
        int pere = (id - 1) / 2;
        MPI_Recv(msg_recu, 2, MPI_INT, pere, 0, MPI_COMM_WORLD, &status);
        printf("Le processus %d a recu [%d, %d] de son pere %d\n", id, msg_recu[0], msg_recu[1], pere);
        
        // On copie dans la variable d'envoi
        val_a_env[0] = msg_recu[0];
        val_a_env[1] = msg_recu[1];

        // Relai
        int fils_gauche = 2 * id + 1;
        int fils_droit = 2 * id + 2;

        if (fils_gauche < n) {
            MPI_Send(val_a_env, 2, MPI_INT, fils_gauche, 0, MPI_COMM_WORLD);
        }
        if (fils_droit < n) {
            MPI_Send(val_a_env, 2, MPI_INT, fils_droit, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}