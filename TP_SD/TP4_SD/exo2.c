#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "mpi.h"

#define d 2

// IMPLÉMENTATION Dijkstra sur graphe aléatoire

/* mpicc -Wall exo2.c -o exo2.exe
    mpirun -np 8 ./exo2.exe
*/

int main(int argc, char** argv) {
    int n, id;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (n <= d) {
        if (id == 0) printf("Erreur : n doit être supérieur à d.\n");
        MPI_Finalize();
        return 1;
    }

// 1- Création du graphe aléatoirement

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

    MPI_Barrier(MPI_COMM_WORLD); 

    // 2 - ALGORITHME DE DIJKSTRA (Avec Pulse/Strate)
    
    int TAG_STRATE   = 10;
    int TAG_PULSE    = 11;
    int TAG_REP      = 12; // Réponse à une strate (Oui/Non)
    int TAG_REMONTEE = 13; // Remonte à la racine pour clore l'étape
    int TAG_FIN      = 14;

    int L = -1;       // Distance, -1 signifie "non visité" (infini)
    int pere = -1; 
    int enfants[n]; 
    int nb_enfants = 0;

    if (id == 0) {
        L = 0;
        pere = 0;
        bool fini = false;
        int E = 1; // Compteur d'étapes
        
        while (!fini) {
            printf("Racine : Envoi de messages de type PULSE/STRATE pour l'étape E = %d\n", E);
            int k_trouves = 0; // Est-ce qu'on a trouvé de nouveaux nœuds à cette étape ?
            
            if (E == 1) {
                // Étape 1 : La racine est directement sur la bordure, elle envoie STRATE
                for (int i = 0; i < nb_voisins; i++) {
                    MPI_Send(&E, 1, MPI_INT, voisins[i], TAG_STRATE, MPI_COMM_WORLD);
                }
                // Récolte des réponses
                for (int i = 0; i < nb_voisins; i++) {
                    int rep;
                    MPI_Recv(&rep, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REP, MPI_COMM_WORLD, &status);
                    if (rep == 1) {
                        enfants[nb_enfants++] = status.MPI_SOURCE;
                        k_trouves = 1;
                    }
                }
            } else {
                // Étape > 1 : La racine envoie PULSE pour réveiller la bordure
                for (int i = 0; i < nb_enfants; i++) {
                    MPI_Send(&E, 1, MPI_INT, enfants[i], TAG_PULSE, MPI_COMM_WORLD);
                }
                // Attente de la fin de la vague
                for (int i = 0; i < nb_enfants; i++) {
                    int rem;
                    MPI_Recv(&rem, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REMONTEE, MPI_COMM_WORLD, &status);
                    if (rem == 1) k_trouves = 1;
                }
            }
            
            // Si aucun nœud n'a été ajouté, l'arbre est fini !
            if (k_trouves == 0 || E == n) {
                fini = true;
                for (int i = 0; i < nb_enfants; i++) {
                    MPI_Send(&E, 1, MPI_INT, enfants[i], TAG_FIN, MPI_COMM_WORLD);
                }
            }
            E++;
        }
    } 
    else {
        int attente_rep = 0;
        int attente_rem = 0;
        int k_trouves = 0;

        while (1) {
            int msg;
            // On utilise ANY_TAG pour éviter les deadlocks si plusieurs voisins nous envoient STRATE en même temps
            MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            int source = status.MPI_SOURCE;
            int tag = status.MPI_TAG;
            
            if (tag == TAG_STRATE) {
                int E_recu = msg;
                if (L == -1) {
                    // Je suis découvert !
                    L = E_recu; 
                    pere = source;
                    int rep = 1; // oui
                    MPI_Send(&rep, 1, MPI_INT, source, TAG_REP, MPI_COMM_WORLD);
                } else {
                    // Je suis déjà dans l'arbre
                    int rep = 0; // non
                    MPI_Send(&rep, 1, MPI_INT, source, TAG_REP, MPI_COMM_WORLD);
                }
            }
            else if (tag == TAG_PULSE) {
                int E_recu = msg;
                k_trouves = 0;
                
                if (L == E_recu - 1) { 
                    // C'est mon tour ! Je suis sur la bordure, je recrute (STRATE)
                    attente_rep = 0;
                    for (int i = 0; i < nb_voisins; i++) {
                        if (voisins[i] != pere) {
                            MPI_Send(&E_recu, 1, MPI_INT, voisins[i], TAG_STRATE, MPI_COMM_WORLD);
                            attente_rep++;
                        }
                    }
                    // Si je suis une feuille absolue (aucun autre voisin que mon père)
                    if (attente_rep == 0) {
                        MPI_Send(&k_trouves, 1, MPI_INT, pere, TAG_REMONTEE, MPI_COMM_WORLD);
                    }
                } 
                else if (L < E_recu - 1) { 
                    // Je suis haut dans l'arbre, je fais juste passer le PULSE vers le bas
                    attente_rem = nb_enfants;
                    if (attente_rem == 0) {
                        MPI_Send(&k_trouves, 1, MPI_INT, pere, TAG_REMONTEE, MPI_COMM_WORLD);
                    } else {
                        for (int i = 0; i < nb_enfants; i++) {
                            MPI_Send(&E_recu, 1, MPI_INT, enfants[i], TAG_PULSE, MPI_COMM_WORLD);
                        }
                    }
                }
            }
            else if (tag == TAG_REP) {
                if (msg == 1) { 
                    enfants[nb_enfants++] = source; 
                    k_trouves = 1; 
                }
                attente_rep--;
                // Si j'ai reçu toutes les réponses de mes voisins, je remonte l'info
                if (attente_rep == 0) {
                    MPI_Send(&k_trouves, 1, MPI_INT, pere, TAG_REMONTEE, MPI_COMM_WORLD);
                }
            }
            else if (tag == TAG_REMONTEE) {
                if (msg == 1) k_trouves = 1;
                attente_rem--;
                // Si j'ai reçu toutes les remontées de mes enfants, je remonte à mon père
                if (attente_rem == 0) {
                    MPI_Send(&k_trouves, 1, MPI_INT, pere, TAG_REMONTEE, MPI_COMM_WORLD);
                }
            }
            else if (tag == TAG_FIN) {
                // L'arbre est terminé, je préviens mes enfants et je m'arrête
                for (int i = 0; i < nb_enfants; i++) {
                    MPI_Send(&msg, 1, MPI_INT, enfants[i], TAG_FIN, MPI_COMM_WORLD);
                }
                break; 
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); 

    // Affichage final
    if (id == 0) {
        printf("L'entité %d est la RACINE de l'arbre Dijkstra.\n", id);
    } else {
        printf("L'entité %d a pour père l'entité %d (Distance E = %d)\n", id, pere, L);
    }

    MPI_Finalize();
    return 0;
}

/* Question 3 :
    *   La valeur de E correspond à la distance maximale
        (le plus court cheminle plus long) qui sépare la racine (l'entité 0)
        du nœud le plus éloigné dans le réseau.
        Exemple :
        Si le nœud le plus lointain du réseau se trouve à 4 sauts de la racine,
        l'onde E=4 va le recruter. L'onde E=5 ne trouvera plus personne (k_trouves = 0)
        et l'algorithme s'arrêtera. La valeur utile de E est donc 4.

    *   La quantité de E est égale a une valeur liée à G qui est l'excentricité du sommet
        racine (le sommet 0) dans le graphe G. On peut aussi l'appeler :
            - La profondeur (ou la hauteur) de l'arbre couvrant généré.
            - Le rayon du graphe (si par chance, l'entité 0 est le centre exact du graphe,
              c'est-à-dire le noeud qui minimise l'excentricité maximale).
*/