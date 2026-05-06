#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#define c 2 // il faut que c x l = nb_processus
#define l 4

// IMPLÉMENTATION : Moyennes au sein d'une grille c x l

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

    float moyenne = somme / nb_voisins;

    printf("Je suis l'entité %d, j'ai pour moyenne %f.\n", id_global, moyenne);

    MPI_Comm_free(&comm_grille);
    MPI_Finalize();
    return 0;
}

/* 1. TOUJOURS FAIRE CORRESPONDRE LA GRILLE AU NOMBRE DE PROCESSUS
 *    - Règle MPI : nb_processus global DOIT être exactement égal à c * l.
 *      Sinon, la fonction MPI_Cart_create fera planter l'exécution.
 *
 * 2. LE PIÈGE DU BORD (MPI_PROC_NULL) ET DE L'INITIALISATION
 *    - Règle MPI : Aux bords de la grille, le voisin vaut MPI_PROC_NULL. 
 *      Un MPI_Recv depuis MPI_PROC_NULL s'exécute instantanément mais 
 *      NE MODIFIE PAS la variable. Si on ne l'initialise pas à 0, on ajoute 
 *      des "valeurs poubelles" de la RAM dans le calcul de la moyenne !
 *
 * 3. LA GÉOMÉTRIE DU DAMIER (RÈGLE DU MIROIR ANTI-DEADLOCK)
 *    - Règle MPI : Si une case (Pair) envoie un message vers son voisin 
 *      du HAUT, ce voisin (Impair) se trouve physiquement au-dessus d'elle. 
 *      Ce voisin doit donc écouter ce qui vient de son BAS ! 
 *      Les paires directionnelles s'inversent toujours : 
 *      (Pair vers Haut  -> Impair écoute Bas)
 *      (Pair vers Droite -> Impair écoute Gauche).
 *
 * 4. LE CALCUL DE LA MOYENNE (NE PAS S'OUBLIER)
 *    - Règle MPI : Si le calcul inclut la valeur du nœud lui-même, il faut 
 *      initialiser le diviseur à 1. Autre bonne pratique : créer une 
 *      variable `somme` séparée pour ne pas écraser sa valeur `p` initiale, 
 *      surtout si le lissage se fait sur plusieurs itérations (boucle for).
 *
 * 5. L'ALÉATOIRE EN PROGRAMMATION PARALLÈLE
 *    - Règle MPI : Tous les processus sont lancés à la même fraction de seconde. 
 *      Ils auront la même "graine" et tireront tous la même valeur p ! 
 *      Il faut TOUJOURS lier la graine à l'identifiant unique : 
 *      "srand(time(NULL) + id_global);"
 *      rand()%(k+ 1) -> pour avoir variable aléatoire comprise entre 0 et k inclu */