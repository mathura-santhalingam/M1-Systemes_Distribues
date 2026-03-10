#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h" //chargement de la librairie MPI

int main(){
    int nb_processus, id;
    MPI_Status status;

    MPI_Init(NULL,NULL); // déclare un communicateur : MPI_COMM_WORLD
    MPI_Comm_size(MPI_COMM_WORLD,&nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);

    if(id == 0) {
        srand(time(NULL));
        int taille_msg = (rand() % 10)+1; // taille aléatoire entre 1 et 10
        int *message = malloc(taille_msg * sizeof(int)); // alloue la mémoire dynamiquement pour ce taille
        printf("L'entité 0 génère un message de taille %d : [ ", taille_msg);
        for (int k = 0; k < taille_msg; k++){
            message[k] = rand() % 100; //on va générer des valeurs aléatoires
            printf("%d ", message[k]);
        }
        printf("]\n");

        // envoi initial à tout le monde sauf lui-même
        int i;
        for (i = 0; i < nb_processus; i++) {
            if (i != id){
                MPI_Send(message, taille_msg, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        free(message);
    // reception
    } else {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // rec info du msg

        int taille_recue;
        MPI_Get_count(&status, MPI_INT, &taille_recue); // met taille du msg dans taille_recue

        int *message_recu = malloc(taille_recue * sizeof(int)); // on alloue dynamiquement l'espace pour le message reçu
        
        MPI_Recv(message_recu, taille_recue, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        
        int expediteur = status.MPI_SOURCE;
        // affichage
        printf("Je suis %d, j'ai reçu %d entiers de %d : [ ", id, taille_recue, status.MPI_SOURCE);
        for(int k=0; k < taille_recue; k++){
            printf("%d ", message_recu[k]);
        }
        printf("]\n");

        // inondation -> envoie à tous les voisins u différents de lui-même et différents de l'expediteur
        for(int u = 0; u < nb_processus; u++) {
            if (u != id && u != expediteur) {
            }
        }
        // Nettoyage de la mémoire
        free(message_recu);
    }
    MPI_Finalize();
    return 0;
    } 
    