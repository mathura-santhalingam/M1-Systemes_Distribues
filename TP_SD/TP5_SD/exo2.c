#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h> // CORRECTION : Utiliser des chevrons pour les librairies standard

/*
   Compilation : mpicc -Wall exo2.c -o exo2.exe
   Exécution   : mpirun -np 8 ./exo2.exe
*/

int main(int argc, char** argv) {
    int id, nb_processus;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    srand(time(NULL) + id);

    // 1. CRÉATION DE LA TOPOLOGIE (MPI_Cart_create)
    MPI_Comm comm;
    int dim = 3;
    int tailles[3] = {2, 2, 2}; // il faut que le produit des trois = nb_processus
    int per = 0;
    int periodes[3]={per, per, per};
    int reorder = 1;
    int it = 5;

    int nb_processus_attendus = tailles[0] * tailles[1] * tailles[2];
    if (nb_processus != nb_processus_attendus) {
        if(id == 0) printf("Erreur, %d processus attendus\n", nb_processus_attendus); 
        MPI_Finalize();
        return 1;
    }

    MPI_Cart_create(MPI_COMM_WORLD, dim, tailles, periodes, reorder, &comm);

    // p doit être un double (les envois et la moyenne demandent des décimales)
    double p = rand()%101;

    // Chercher les voisins immédiats
    int v_gauche, v_droite, v_bas, v_haut, v_arriere, v_avant;

    // Dimension X : gauche et droite :
    MPI_Cart_shift(comm, 0, 1, &v_gauche, &v_droite);
    // Dimension Y : bas et haut :
    MPI_Cart_shift(comm, 1, 1, &v_bas, &v_haut);
    // Dimension Z : arrière et avant :
    MPI_Cart_shift(comm, 2, 1, &v_arriere, &v_avant);

    // MOYENNE : ÉCHANGE D'INFOS
    int coords[3];
    MPI_Cart_coords(comm, id, 3, coords); 
    // Si la somme des coordonnées est divisible par 2, on est "pair"
    int est_pair = ((coords[0] + coords[1] + coords[2]) % 2 == 0);

    for (int step = 0; step < it; step++) {
        double p_vd = 0, p_vg = 0, p_vh = 0, p_vb = 0, p_vav = 0, p_var = 0;

        // --- DIMENSION 0 (Axe X : Gauche / Droite) ---
        if (est_pair) {
            // Le pair initie l'échange vers la droite
            MPI_Send(&p, 1, MPI_DOUBLE, v_droite, 0, comm);
            MPI_Recv(&p_vg, 1, MPI_DOUBLE, v_gauche, 0, comm, MPI_STATUS_IGNORE);
            
            // Le pair initie l'échange vers la gauche
            MPI_Send(&p, 1, MPI_DOUBLE, v_gauche, 0, comm);
            MPI_Recv(&p_vd, 1, MPI_DOUBLE, v_droite, 0, comm, MPI_STATUS_IGNORE);
        } else {
            // L'impair attend le message venant de la gauche (envoyé par le pair)
            MPI_Recv(&p_vg, 1, MPI_DOUBLE, v_gauche, 0, comm, MPI_STATUS_IGNORE);
            MPI_Send(&p, 1, MPI_DOUBLE, v_droite, 0, comm);
            
            // L'impair attend le message venant de la droite
            MPI_Recv(&p_vd, 1, MPI_DOUBLE, v_droite, 0, comm, MPI_STATUS_IGNORE);
            MPI_Send(&p, 1, MPI_DOUBLE, v_gauche, 0, comm);
        }

        // --- DIMENSION 1 (Axe Y : Bas / Haut) ---
        if (est_pair) {
            MPI_Send(&p, 1, MPI_DOUBLE, v_haut, 0, comm);
            MPI_Recv(&p_vb, 1, MPI_DOUBLE, v_bas, 0, comm, MPI_STATUS_IGNORE);

            MPI_Send(&p, 1, MPI_DOUBLE, v_bas, 0, comm);
            MPI_Recv(&p_vh, 1, MPI_DOUBLE, v_haut, 0, comm, MPI_STATUS_IGNORE);
        } else {
            MPI_Recv(&p_vb, 1, MPI_DOUBLE, v_bas, 0, comm, MPI_STATUS_IGNORE);
            MPI_Send(&p, 1, MPI_DOUBLE, v_haut, 0, comm);

            MPI_Recv(&p_vh, 1, MPI_DOUBLE, v_haut, 0, comm, MPI_STATUS_IGNORE);
            MPI_Send(&p, 1, MPI_DOUBLE, v_bas, 0, comm);
        }

        // --- DIMENSION 2 (Axe Z : Arrière / Avant) ---
        if (est_pair) {
            MPI_Send(&p, 1, MPI_DOUBLE, v_avant, 0, comm);
            MPI_Recv(&p_var, 1, MPI_DOUBLE, v_arriere, 0, comm, MPI_STATUS_IGNORE);

            MPI_Send(&p, 1, MPI_DOUBLE, v_arriere, 0, comm);
            MPI_Recv(&p_vav, 1, MPI_DOUBLE, v_avant, 0, comm, MPI_STATUS_IGNORE);
        } else {
            MPI_Recv(&p_var, 1, MPI_DOUBLE, v_arriere, 0, comm, MPI_STATUS_IGNORE);
            MPI_Send(&p, 1, MPI_DOUBLE, v_avant, 0, comm);

            MPI_Recv(&p_vav, 1, MPI_DOUBLE, v_avant, 0, comm, MPI_STATUS_IGNORE);
            MPI_Send(&p, 1, MPI_DOUBLE, v_arriere, 0, comm);
        }

        // Calcul de la moyenne (inchangé)
        double somme = p;
        int nb_valeurs = 1; 

        if (v_gauche  != MPI_PROC_NULL) { somme += p_vg;  nb_valeurs++; }
        if (v_droite  != MPI_PROC_NULL) { somme += p_vd;  nb_valeurs++; }
        if (v_bas     != MPI_PROC_NULL) { somme += p_vb;  nb_valeurs++; }
        if (v_haut    != MPI_PROC_NULL) { somme += p_vh;  nb_valeurs++; }
        if (v_arriere != MPI_PROC_NULL) { somme += p_var; nb_valeurs++; }
        if (v_avant   != MPI_PROC_NULL) { somme += p_vav; nb_valeurs++; }

        p = somme / nb_valeurs;
    }

    if (id == 0) {
        printf("Simulation terminée avec damier. Valeur lissée : %f\n", p);
    }

    MPI_Comm_free(&comm);
    MPI_Finalize();
    return 0;
}