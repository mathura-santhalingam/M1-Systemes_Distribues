#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char** argv){

    int n, id_global, msg;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);

    if (id_global == 1) {
        msg = 42; 
    }
    // question a)
    MPI_Comm nv_comm;
    int couleur = id_global %2;
    int nv_id = (id_global - id_global %2) / 2;
    MPI_Comm_split(MPI_COMM_WORLD, couleur, nv_id, &nv_comm);

    if (couleur == 1) {
        MPI_Bcast(&msg, 1, MPI_INT, 0, nv_comm); // attention root = 0 car id_global 1 = 0 dans nv_comm puisque c'est le premier impair
    }
    MPI_Comm_free(&nv_comm);

    // question b) avec n=12
    MPI_Comm nv_comm;
    int dims[2]={4,3};
    int periodes[2]={1,0};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodes, 1, &nv_comm);

    /* Le nombre de voisins dépend de la position de l'entité sur la deuxième dimension (l'axe non-périodique) :
        - Les entités situées sur les bords de la deuxième dimension :
            Elles ont 2 voisins (axe périodique) + 1 voisin (axe non-périodique) = 3 voisins.
            (Cela concerne 8 entités : les 4 de la première "tranche" et les 4 de la dernière "tranche").

        - Les entités situées au milieu de la deuxième dimension :
            Elles ont 2 voisins (axe périodique) + 2 voisins (axe non-périodique) = 4 voisins.
            (Cela concerne les 4 entités de la tranche centrale).

    Donc en résumé : Dans ce système, 8 entités ont 3 voisins et 4 entités ont 4 voisins.*/

    // question d)
    int p;
    MPI_Status status;

    srand(time(NULL) + id_global);
    p = rand() % 100;

    int haut, bas, gauche, droite;
    MPI_Cart_shift(nv_comm, 0, 1, &haut, &bas);
    MPI_Cart_shift(nv_comm, 1, 1, &gauche, &droite);

    int coord[2];
    MPI_Cart_coords(nv_comm, id_global, 2, coord);

    int p_haut = -1, p_bas = -1, p_gauche = -1, p_droite = -1;

    if ((coord[0] + coord[1]) % 2 == 0) {
        // Les cases "Paires" ENVOIENT d'abord, puis REÇOIVENT
        MPI_Send(&p, 1, MPI_INT, haut, 0, nv_comm);
        MPI_Recv(&p_haut, 1, MPI_INT, haut, 0, nv_comm, &status);
        
        MPI_Send(&p, 1, MPI_INT, bas, 0, nv_comm);
        MPI_Recv(&p_bas, 1, MPI_INT, bas, 0, nv_comm, &status);
        
        MPI_Send(&p, 1, MPI_INT, gauche, 0, nv_comm);
        MPI_Recv(&p_gauche, 1, MPI_INT, gauche, 0, nv_comm, &status);
        
        MPI_Send(&p, 1, MPI_INT, droite, 0, nv_comm);
        MPI_Recv(&p_droite, 1, MPI_INT, droite, 0, nv_comm, &status);
    } else {
        // Les cases "Impaires" REÇOIVENT d'abord, puis ENVOIENT
        MPI_Recv(&p_bas, 1, MPI_INT, bas, 0, nv_comm, &status);
        MPI_Send(&p, 1, MPI_INT, bas, 0, nv_comm);

        MPI_Recv(&p_haut, 1, MPI_INT, haut, 0, nv_comm, &status);
        MPI_Send(&p, 1, MPI_INT, haut, 0, nv_comm);
        
        MPI_Recv(&p_droite, 1, MPI_INT, droite, 0, nv_comm, &status);
        MPI_Send(&p, 1, MPI_INT, droite, 0, nv_comm);
        
        MPI_Recv(&p_gauche, 1, MPI_INT, gauche, 0, nv_comm, &status);
        MPI_Send(&p, 1, MPI_INT, gauche, 0, nv_comm);
    }
    int max_voisins = -1; 

    // On vérifie que le voisin existe (!= MPI_PROC_NULL) avant de comparer
    if (haut != MPI_PROC_NULL && p_haut > max_voisins) max_voisins = p_haut;
    if (bas != MPI_PROC_NULL && p_bas > max_voisins) max_voisins = p_bas;
    if (gauche != MPI_PROC_NULL && p_gauche > max_voisins) max_voisins = p_gauche;
    if (droite != MPI_PROC_NULL && p_droite > max_voisins) max_voisins = p_droite;

    if (max_voisins != -1) {
        p = max_voisins;
    }

    MPI_Finalize();
    return 0;
}