#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define d 2 // Nombre d minimum de voisins choisis

// IMPLÉMENTATION FORD-BELLMAN

/* mpicc -Wall exo1.c -o exo1.exe
    mpirun -np 8 ./exo1.exe
*/

// 1- Création du graphe aléatoirement
int main(int argc, char** argv) {
    int n, id;
    int msg_recu;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n <= d) {
        if (id == 0) printf("Erreur : n doit être supérieur à d.\n");
        MPI_Finalize();
        return 1;
    }

    int voisins[n-1];
    int nb_voisins = 0;
    int mes_choix[n];
    
    for (int j = 0; j < n; j++) {
        mes_choix[j] = 0; 
    }

    srand(time(NULL) + id);
    int choisis = 0;
    
    while (choisis < d) {
        int r = rand() % n;
        if (r != id && mes_choix[r] == 0) {
            mes_choix[r] = 1; 
            voisins[nb_voisins] = r; 
            nb_voisins++;
            choisis++;
        }
    }

    for (int j = 0; j < n; j++) {
        if (j != id) { 
            int msg_envoye = mes_choix[j]; 
            int msg_recu;
            
            if (id < j) {
                MPI_Send(&msg_envoye, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                MPI_Recv(&msg_recu, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &status);
            } else {
                MPI_Recv(&msg_recu, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &status);
                MPI_Send(&msg_envoye, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            }

            if (msg_recu == 1 && mes_choix[j] == 0) {
                voisins[nb_voisins] = j;
                nb_voisins++;
            }
        }
    }

    // On synchronise tout le monde avant de lancer Bellman-Ford
    // Ça garantit que tous les messages de création du graphe sont consommés
    MPI_Barrier(MPI_COMM_WORLD); 

    // 2- ALGORITHME DE BELLMAN-FORD (Arbre de parcours en largeur)
    
    // Définition de balises spécifiques pour ne pas confondre avec l'étape 1
    int TAG_DONNEE = 10;
    int TAG_ACK    = 11;
    int TAG_PERE   = 12;

    int l = n + 1; // Distance (L(u)), initialisée à l'infini (n+1)
    int pere = -1; // PERE(u)
    int c = 0;     // Compteur de terminaison (c)

    if (id == 0) {
        l = 0;
        pere = 0; // La racine est son propre père (ou pas de père)
        c = 2 * nb_voisins;

        // Diffuser la distance 0 à tous ses voisins
        for (int i = 0; i < nb_voisins; i++) {
            MPI_Send(&l, 1, MPI_INT, voisins[i], TAG_DONNEE, MPI_COMM_WORLD);
        }
    } else {
        int d_recu;
        // On reçoit le message de n'importe quelle source
        MPI_Recv(&d_recu, 1, MPI_INT, MPI_ANY_SOURCE, TAG_DONNEE, MPI_COMM_WORLD, &status);

        int v = status.MPI_SOURCE; 

        l = d_recu + 1;
        pere = v;

        c = 2 * nb_voisins - 2;

        // Transmettre la nouvelle distance à tous les voisins SAUF au père (v)
        for (int i = 0; i < nb_voisins; i++) {
            if (voisins[i] != pere) { // car attention on est pas dans un graphe complet
                MPI_Send(&l, 1, MPI_INT, voisins[i], TAG_DONNEE, MPI_COMM_WORLD);
            }
        }
    }

    while (c > 0) {
        MPI_Recv(&msg_recu, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;
        int tag = status.MPI_TAG;

        if (tag == TAG_DONNEE) {
            int d_recu = msg_recu;

            if (l > d_recu + 1) {
                // on a trouvé un meilleur chemin
                int msg_donnee = d_recu + 1;
                MPI_Send(&msg_donnee, 1, MPI_INT, pere, TAG_DONNEE, MPI_COMM_WORLD);
                
                int ack = 1;
                MPI_Send(&ack, 1, MPI_INT, pere, TAG_ACK, MPI_COMM_WORLD);
                
                l = d_recu + 1;
                pere = source; // On met à jour notre nouveau père
            } else {
                // Le chemin n'est pas meilleur, on libère l'expéditeur
                int ack = 1;
                MPI_Send(&ack, 1, MPI_INT, source, TAG_ACK, MPI_COMM_WORLD);
            }
            c--; // On a traité un message DONNEE
        }
        else if (tag == TAG_PERE) {
            // L'expéditeur confirme qu'il est notre fils
            c -= 2; 
        }
        else if (tag == TAG_ACK) {
            // L'expéditeur a bien reçu notre donnée
            c--;
        }
    }

    // À la fin de la boucle, le nœud (sauf la racine) informe son père définitif
    if (id != 0) {
        int msg_pere = 1;
        MPI_Send(&msg_pere, 1, MPI_INT, pere, TAG_PERE, MPI_COMM_WORLD);
    }

    // On synchronise l'affichage pour que la console soit propre
    MPI_Barrier(MPI_COMM_WORLD); 

    // Affichage final demandé par la question 1
    if (id == 0) {
        printf("L'entité %d est la RACINE du graphe.\n", id);
    } else {
        printf("L'entité %d a pour père l'entité %d (Distance = %d)\n", id, pere, l);
    }

    MPI_Finalize();
    return 0;
}