#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#define c 2 // il faut que c x l = nb_processus
#define l 4

// IMPLÉMENTATION : Diffusion de p dans une grille c x l (CART_SUB + BCAST)

/*
   Compilation : mpicc -Wall exo2_td4.c -o exo2_td4.exe
   Exécution   : mpirun -np 8 ./exo2_td4.exe
*/

int main (int argc, char** argv){
    int nb, id_global;
    int msg_vd = 0, msg_vg = 0, msg_vh = 0, msg_vb = 0;
    int nb_voisins = 1;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    if (nb != c * l) {
        if (id_global == 0) printf("Erreur: %d processus requis pour une grille %dx%d\n", c*l, c, l);
        MPI_Finalize();
        return 1;
    }

    srand(time(NULL)+ id_global);
    int p = rand()%100;

    // création de la grille 2D
    MPI_Comm comm_grille;
    int dims[2] = {c,l};
    int periode[2] = {0,0};
    int reorder = 1;

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periode, reorder, &comm_grille);

    // définir les voisins :
    int gauche, droit, haut, bas;

    MPI_Cart_shift(comm_grille, 0, 1, &haut, &bas);// 0 : ligne, 1 : colonne
    MPI_Cart_shift(comm_grille, 1, 1, &gauche, &droit);

    // calcul des moyennes nécessite échange de messages
    // échange de message nécessite de savoir qui l'initie => parité de la somme des coordonnées
    int coord[2];
    MPI_Cart_coords(comm_grille, id_global, 2, coord);

    // calcul des moyennes
    if((coord[0]+coord[1])%2 == 0) { // les entités de somme de coord paires initient l'échange
        MPI_Send(&p, 1, MPI_INT, haut, 0, comm_grille); // envoie à haut
        MPI_Recv(&msg_vh, 1, MPI_INT, haut, 0, comm_grille, &status);

        MPI_Send(&p, 1, MPI_INT, bas, 0, comm_grille);
        MPI_Recv(&msg_vb, 1, MPI_INT, bas, 0, comm_grille, &status);

        MPI_Send(&p, 1, MPI_INT, gauche, 0, comm_grille);
        MPI_Recv(&msg_vg, 1, MPI_INT, gauche, 0, comm_grille, &status);

        MPI_Send(&p, 1, MPI_INT, droit, 0, comm_grille);
        MPI_Recv(&msg_vd, 1, MPI_INT, droit, 0, comm_grille, &status);

    } else {

        MPI_Recv(&msg_vb, 1, MPI_INT, bas, 0, comm_grille, &status); // reçoit de bas
        MPI_Send(&p, 1, MPI_INT, bas, 0, comm_grille);

        MPI_Recv(&msg_vh, 1, MPI_INT, haut, 0, comm_grille, &status);
        MPI_Send(&p, 1, MPI_INT, haut, 0, comm_grille);

        MPI_Recv(&msg_vd, 1, MPI_INT, droit, 0, comm_grille, &status);
        MPI_Send(&p, 1, MPI_INT, droit, 0, comm_grille);

        MPI_Recv(&msg_vg, 1, MPI_INT, gauche, 0, comm_grille, &status);
        MPI_Send(&p, 1, MPI_INT, gauche, 0, comm_grille);
    }

    int somme = p;

    if (haut != MPI_PROC_NULL) { somme += msg_vh; nb_voisins++;}
    if (bas != MPI_PROC_NULL) { somme += msg_vb; nb_voisins++;}
    if (gauche != MPI_PROC_NULL) { somme += msg_vg; nb_voisins++;}
    if (droit != MPI_PROC_NULL) { somme += msg_vd; nb_voisins++;}

    double moyenne = (double) somme / nb_voisins;

    // Diffusion le long des lignes

    MPI_Comm comm_lignes;
    int d_restantes_lignes[2] = {0,1}; // le long des lignes => ligne bouge pas et on avance/diffuse en avançant par colonne
    MPI_Cart_sub(comm_grille, d_restantes_lignes, &comm_lignes);

    MPI_Bcast(&moyenne, 1, MPI_DOUBLE, 0, comm_lignes); // on peut directement dire root = 0 puisque Cart_sub remappe
    MPI_Comm_free(&comm_lignes);
    
    // Diffusion le long des colonnes
        
    MPI_Comm comm_col;
    int d_restantes_col[2] = {1,0};
    MPI_Cart_sub(comm_grille, d_restantes_col, &comm_col);

    MPI_Bcast(&moyenne, 1, MPI_DOUBLE, 0, comm_col);
    MPI_Comm_free(&comm_col);

    MPI_Comm_free(&comm_grille);
    MPI_Finalize();
    return 0;
}