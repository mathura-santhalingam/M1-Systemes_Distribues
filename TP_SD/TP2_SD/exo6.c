#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define d 2 // "nombre d choisi avec une directive define"

/*  mpicc -Wall exo6.c -o exo6.exe
    mpirun -np 8 ./exo6.exe
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

    int voisins[n-1];
    int nb_voisins = 0;

    int mes_choix[n];
    for (int j = 0; j < n; j++) {
        mes_choix[j] = 0; 
    }

    // Choisir d entités distinctes comme voisins
    srand(time(NULL) + id);
    int choisis = 0;
    
    while (choisis < d) { // tant qu'on a pas d voisins
        int r = rand() % n;
        if (r != id && mes_choix[r] == 0) { // si ni moi, et  ni déjà voisin, j'ajoute cette valeur aléatoire à ma liste de voisins
            mes_choix[r] = 1; 
            voisins[nb_voisins] = r; 
            nb_voisins++;
            choisis++;
        }
    }

    // La communication SANS Sendrecv, pour savoir qui nous a choisi
    // Attention : une entité de calcul doit avoir au moins d voisins mais peut en avoir plus que d dans le cas où A choisi C et D comme voisins et B choisi A comme voisin => A a pour voisins B, C et D
    for (int j = 0; j < n; j++) { // boucle for car chaque entité va parler une par une à toutes les autres
        if (j != id) { 
            int msg_envoye = mes_choix[j]; // comme on envoie message à tlm, on leur donne ça et non notre id
            int msg_recu;
            
            // LA STRATÉGIE ANTI-DEADLOCK :
            if (id < j) {
                // Si mon ID est plus petit, je prends l'initiative de parler
                MPI_Send(&msg_envoye, 1, MPI_INT, j, 0, MPI_COMM_WORLD); // Je lui envoie le contenu de mon annuaire le concernant : 1 (je t'ai invité) ou 0 (je ne t'ai pas invité).
                MPI_Recv(&msg_recu, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &status);
            } 
            else { // id > j
                // Si mon ID est plus grand, je le laisse parler en premier
                MPI_Recv(&msg_recu, 1, MPI_INT, j, 0, MPI_COMM_WORLD, &status);
                MPI_Send(&msg_envoye, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            }

            // Mise à jour de la liste des voisins (comme avant)
            if (msg_recu == 1 && mes_choix[j] == 0) {
                voisins[nb_voisins] = j;
                nb_voisins++;
            }
        }
    }

    // Affichage de contrôle
    printf("Entite %d a finalement %d voisins : ", id, nb_voisins);
    for (int i = 0; i < nb_voisins; i++) {
        printf("%d ", voisins[i]);
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

/*Stratégie anti deadlock : attention la parité ne marche que si on est sûr
qu'un processus pair ne parlera pas à un autre processus pair (ex ping-pong entre 2 entité ou anneau à nb processus total pair).
Donc ici ça n'aurait pas marché puisque tout le monde parle avec tout le monde (for) y compris :
    Le processus 2 veut parler au processus 4.
    Le processus 2 (pair) fait un MPI_Send.
    Le processus 4 (pair) fait un MPI_Send.
    Résultat : Interblocage (Deadlock)

Solution de " < ": La règle if (id < j) est infaillible car elle crée un ordre strict pour n'importe quelle paire de processus dans le monde.
    Il est mathématiquement impossible que deux processus soient tous les deux plus petits que l'autre.
    Il y aura toujours un gagnant (qui fera Send) et un perdant (qui fera Recv).
    PARFAIT quand tout le monde veut parler avec tout le monde dans le désordre
*/