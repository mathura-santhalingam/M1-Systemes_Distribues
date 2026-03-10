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
        // envoi
        int i;
        for (i = 0; i < nb_processus; i++) {
            if (i != id){
                MPI_Send(message, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    // reception
    } else {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // rec taille du msg
        //printf("Je suis %d et j'envoie la valeur %d à %d.\n", identifiant, k, (identifiant+1)%2);
        //MPI_Send(&k,1,MPI_INT,((identifiant+1)%2),0,MPI_COMM_WORLD);
        int taille;
        MPI_Get_count(&status, MPI_INT, &taille);
        //printf("taille = %d\n", taille);
        MPI_Recv(message, taille, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        printf("Je suis %d et j'ai reçu [%d,%d] de %d.\n", id, message[0],message[1], status.MPI_SOURCE);
        
        int expediteur = status.MPI_SOURCE;

        // envoyer message à tous les voisins u différents de 0 et différents de l'expediteur
        for(int u = 0; u < nb_processus; u++) {
            if (u != id && u != expediteur) {
                MPI_Send(message, 2, MPI_INT, u, 1, MPI_COMM_WORLD);
            }
        }
    }
    MPI_Finalize();
    return 0;
    } 
    