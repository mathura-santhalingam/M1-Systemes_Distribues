#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// IMPLÉMENTATION : diffuser un msg à partir de l'entité 1 vers toutes les autres entités d'identifiant impair et uniquement celles-là (Bcast)

int main(int argc, char** argv){
    int msg;
    int n, id_global;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    MPI_Comm comm_grille;

    int nv_id = (id_global -(id_global % 2) ) /2;

    int couleur = (id_global % 2 == 1) ? 0 : MPI_UNDEFINED;
    int p, rang_local;
    MPI_Status status;
    MPI_Comm_rank(comm_grille, &rang_local);

    // i. Affectation
    srand(time(NULL) + rang_local);
    p = rand() % 100;

    // Récupération des voisins et des coordonnées
    int ht, bs, g, d;
    MPI_Cart_shift(comm_grille, 0, 1, &ht, &bs);
    MPI_Cart_shift(comm_grille, 1, 1, &g, &d);
    
    int coord[2];
    MPI_Cart_coords(comm_grille, rang_local, 2, coord);

    int p_recu;
    int max_voisins = -1;
    int voisins[4]; // Le tableau qui va raccourcir notre code !

    // ii. L'échange avec boucle FOR et Effet Miroir intégré
    if ((coord[0] + coord[1]) % 2 == 0) {
        
        // PAIRS : Ordre classique
        voisins[0] = ht; voisins[1] = bs; voisins[2] = g; voisins[3] = d;
        
        for (int i = 0; i < 4; i++) {
            if (voisins[i] != MPI_PROC_NULL) {
                MPI_Send(&p, 1, MPI_INT, voisins[i], 0, comm_grille);
                MPI_Recv(&p_recu, 1, MPI_INT, voisins[i], 0, comm_grille, &status);
                
                // Traitement à la volée
                if (p_recu > max_voisins) max_voisins = p_recu;
            }
        }
        
    } else {
        
        // IMPAIRS : Ordre miroir (Bas répond à Haut, etc.)
        voisins[0] = bs; voisins[1] = ht; voisins[2] = d; voisins[3] = g;
        
        for (int i = 0; i < 4; i++) {
            if (voisins[i] != MPI_PROC_NULL) {
                MPI_Recv(&p_recu, 1, MPI_INT, voisins[i], 0, comm_grille, &status);
                MPI_Send(&p, 1, MPI_INT, voisins[i], 0, comm_grille);
                
                // Traitement à la volée
                if (p_recu > max_voisins) max_voisins = p_recu;
            }
        }
    }

    // iii. Remplacement final
    if (max_voisins != -1) {
        p = max_voisins;
    }

    MPI_Finalize();
    return 0;
}