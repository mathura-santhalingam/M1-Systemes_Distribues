#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h" //chargement de la librairie MPI

int main(){
    int nb_processus, id;
    int message[2]; // car le message contient 2 entiers
    MPI_Status status;

    MPI_Init(NULL,NULL); // déclare un communicateur : MPI_COMM_WORLD
    MPI_Comm_size(MPI_COMM_WORLD,&nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);

    if(id == 0) {
        //on va générer des valeurs aléatoires
        srand(time(NULL));
        message[0] = rand() % 100;
        message[1] = rand() % 100;
        printf("L'entité 0 a généré le message [%d, %d]\n", message[0],message[1]);
    }
    // Chacun des processus devra afficher le message reçu
    else {
        MPI_Recv(&message,2,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        printf("Je suis %d et j'ai reçu [%d,%d] de %d.\n", id, message[0],message[1], status.MPI_SOURCE);
        //printf("Je suis %d et j'envoie la valeur %d à %d.\n", identifiant, k, (identifiant+1)%2);
        //MPI_Send(&k,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
    }
    int fg = 2 * id + 1;
    int fd  = 2 * id + 2;
    if (fg < nb_processus) {
        MPI_Send(message, 2, MPI_INT, fg, 0, MPI_COMM_WORLD);
    }
    if (fd < nb_processus) {
        MPI_Send(message, 2, MPI_INT, fd, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}