#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*
   Compilation : mpicc -Wall split.c -o split.exe
   Exécution   : mpirun -np 7 ./split.exe
*/

int main(int argc, char** argv) {
    int id_global, nb_global;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_global);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_global);

    // 1. LE DÉCOUPAGE (La formule de ton cours)
    MPI_Comm comm_pair_impair;
    
    // La couleur détermine le groupe (0 pour pair, 1 pour impair)
    int couleur = id_global % 2;
    
    int nouvel_id = (id_global - id_global % 2) / 2; // ce qui donnera 0 1 2 3 4 5 -> 0 0 2 2 4 4 -> 0 0 1 1 2 2

    // toutes les entités de même couleur partent dans le même groupe
    // ATTENTION on a fait couleur AVANT nouvel_id donc on se base sur la couleur des id_global donc 0 1 2 d'un côté et 0 1 2 de l'autre
    MPI_Comm_split(MPI_COMM_WORLD, couleur, nouvel_id, &comm_pair_impair);

    // 2. INIT NOUVELLE COMM
    int id_local, nb_local;
    MPI_Comm_rank(comm_pair_impair, &id_local); // égal à nouvel_id
    MPI_Comm_size(comm_pair_impair, &nb_local); // égal au nb de processus dans le groupe courant

    printf("Je suis le global %d. Dans mon nouveau groupe (couleur %d), je suis l'entité %d sur %d.\n", id_global, couleur, id_local, nb_local);

    MPI_Barrier(MPI_COMM_WORLD); // Juste pour que l'affichage console soit propre
    if(id_global == 0) printf("\nDÉBUT DE L'OPÉRATION COLLECTIVE ISOLÉE.\n\n");
    MPI_Barrier(MPI_COMM_WORLD);

    // 3. LA PREUVE DE L'UTILITÉ : Une action collective isolée
    int ma_valeur = id_global; // Chacun prépare son ID d'origine
    int somme_groupe = 0;

    // Ce Reduce se fait sur le NOUVEAU communicateur !
    // Les pairs vont additionner leurs IDs entre eux, et les impairs de leur côté.
    MPI_Reduce(&ma_valeur, &somme_groupe, 1, MPI_INT, MPI_SUM, 0, comm_pair_impair);

    // Seul le "chef" (id_local == 0) de CHAQUE groupe affiche le résultat
    if (id_local == 0) {
        if (couleur == 0) {
            printf(">> GROUPE PAIR   : La somme de nos anciens IDs est %d\n", somme_groupe);
        } else {
            printf(">> GROUPE IMPAIR : La somme de nos anciens IDs est %d\n", somme_groupe);
        }
    }

    // On libère le communicateur créé
    MPI_Comm_free(&comm_pair_impair);

    MPI_Finalize();
    return 0;
}

/*  Si tout le monde est dans MPI_COMM_WORLD, un MPI_Barrier() ou un MPI_Bcast() va bloquer
    et mélanger tout le monde.
    Grâce à MPI_Comm_split, on crée des sous-groupes isolés (de nouveaux communicateurs).
    Ainsi, les processus pairs peuvent faire un MPI_Reduce entre eux,
    pendant que les impairs font totalement autre chose, sans jamais interférer.

    Le communicateur nouveau_comm doit être déclaré sur chaque entité de calcul,
    mais sa valeur après l’appel de MPI_Comm_split sera différente d’une entité de
    calcul à l’autre. Toutes les entités de calcul qui ont la même couleur (un entier)
    seron regroupées dans le même nouveau communicateur.
*/