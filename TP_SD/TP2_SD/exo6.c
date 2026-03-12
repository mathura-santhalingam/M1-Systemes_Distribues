#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define d 2 

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

    // Choisir d entités distinctes
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
    for (int j = 0; j < n; j++) { // boucle for car chaque entité va parler une par une à toutes les autres
        if (j != id) { 
            int msg_envoye = mes_choix[j]; 
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