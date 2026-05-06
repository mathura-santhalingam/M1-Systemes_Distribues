#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*
   Compilation : mpicc -Wall topo.c -o topo.exe
   Exécution   : mpirun -np 8 ./topo.exe
*/

int main(int argc, char** argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 8) {
        if (rank == 0) printf("Erreur : Ce test nécessite exactement 8 processus.\n");
        MPI_Finalize();
        return 1;
    }

    // 1. CRÉATION DE LA TOPOLOGIE (MPI_Cart_create)
    MPI_Comm comm_grille;
    int ndims = 2;               // On veut une grille en 2 dimensions (lignes/colonnes)
    int dimensions[2] = {2, 4};  // 2 lignes (dim 0) et 4 colonnes (dim 1)
    int periodes[2] = {0, 0};    // 0 = grille non-périodique (pas un tore, il y a des bords)
    int reorder = 1;             // MPI a le droit de réordonner les rangs pour optimiser

    /*  -----------------
        | 0 | 1 | 2 | 3 |
        | 4 | 5 | 6 | 7 |
        -----------------> dim 1
        v
        dim 0                    */

    // Création du nouveau communicateur structuré en grille 2D
    MPI_Cart_create(MPI_COMM_WORLD, ndims, dimensions, periodes, reorder, &comm_grille);

    // 2. RECHERCHE DE COORDONNÉES (MPI_Cart_coords)
    int coords[2];
    // Remplissage du tableau 'coords' avec la position x,y du processus courant
    MPI_Cart_coords(comm_grille, rank, ndims, coords);

    // Pour éviter que l'affichage console soit chaotique, seul le rang 6 va parler pour l'exemple
    if (rank == 6) {
        printf("--- SECTION 2 : COORDONNÉES (Pour l'entité 6) ---\n");
        // Devrait afficher (1, 2) comme dans ton cours
        printf("L'entité %d se trouve aux coordonnées (%d, %d).\n\n", rank, coords[0], coords[1]);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 3. RECHERCHE DE VOISINS (MPI_Cart_shift)
    if (rank == 6) {
        printf("--- SECTION 3 : VOISINAGE (Pour l'entité 6) ---\n");
        int voisin_prec, voisin_suiv;

        // A. Recherche sur l'axe HORIZONTAL (Les colonnes, dimension 1)
        // Le 2e argument est la dimension (1 pour les colonnes), le 3e est le "pas" (1 pour le voisin direct).
        MPI_Cart_shift(comm_grille, 1, 1, &voisin_prec, &voisin_suiv);
        // Devrait retourner prec=5 et suiv=7
        printf("Axe Horizontal (Dim 1) -> prec : %d | suiv : %d\n", voisin_prec, voisin_suiv);


        // B. Recherche sur l'axe VERTICAL (Les lignes, dimension 0)
        MPI_Cart_shift(comm_grille, 0, 1, &voisin_prec, &voisin_suiv);
        // Le voisin du haut ('prec') est 2. 
        // Le voisin du bas ('suiv') n'existe pas, car 6 est sur la ligne du bas.
        // MPI renvoie MPI_PROC_NULL (-3 généralement) pour éviter les crashs.

        printf("Axe Vertical   (Dim 0) -> Haut   : %d | Bas    : ", voisin_prec);
        if (voisin_suiv == MPI_PROC_NULL) {
            printf("MPI_PROC_NULL (Bord de la grille !)\n\n");
        } else {
            printf("%d\n\n", voisin_suiv);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);


    // 4. SOUS-COMMUNICATEURS DIMENSIONNELS (MPI_Cart_sub)
    MPI_Comm comm_ligne;
    
    // Le tableau d_restantes indique quelles dimensions on garde libres.
    // En MPI standard : {0, 1} signifie que la dimension 0 (les lignes) est FIXÉE, 
    // et que la dimension 1 (les colonnes) reste LIBRE. 
    // Cela regroupe donc tous les éléments d'une même ligne dans un nouveau communicateur.
    int d_restantes[2] = {0, 1}; 
    
    // Création des communicateurs par lignes
    MPI_Cart_sub(comm_grille, d_restantes, &comm_ligne);

    // On récupère le nouveau rang de notre entité au sein de sa propre ligne
    int rank_dans_ligne;
    MPI_Comm_rank(comm_ligne, &rank_dans_ligne);

    // Pour isoler des lignes, il faut figer l'axe vertical (mettre 0 à la dimension 0)

    // On fait parler l'entité 6 pour voir ce qui s'est passé :
    if (rank == 6) {
         printf("--- SECTION 4 : SOUS-COMMUNICATEURS ---\n");
         // Dans la ligne du haut (4, 5, 6, 7), l'entité globale 6 est la 3ème entité.
         // Son nouveau rang local sera donc 2.
         printf("Je suis le global 6. Dans mon sous-communicateur de ligne, je suis le rang local %d.\n", rank_dans_ligne);
    }

    // Libération de la mémoire
    MPI_Comm_free(&comm_ligne);
    MPI_Comm_free(&comm_grille);

    MPI_Finalize();
    return 0;
}