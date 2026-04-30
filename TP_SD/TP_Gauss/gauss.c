#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define NL 4       // Nombre de lignes/inconnues de la matrice A
#define NC (NL+1)  // Nombre de colonnes (Matrice A + vecteur b)

/*
   Compilation : mpicc -Wall gauss.c -o gauss.exe
   Exécution   : mpirun -np 2 ./gauss.exe
*/

int main(int argc, char** argv) {
    int id, nb_processus;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus);

    if (NL % nb_processus != 0) {
        if (id == 0) printf("Erreur: NL (%d) doit être divisible par le nombre d'entités (%d).\n", NL, nb_processus);
        MPI_Finalize();
        return 1;
    }

    int k = NL / nb_processus; // Nombre de lignes gérées par chaque entité
    int M[k][NC]; // La sous-matrice locale de chaque entité

    // Génération d'une matrice de valeurs arbitraires sauf pour les 2 premières entités pour tester.
    srand(time(NULL) + id);
    if (id == 0 || id == 1) {
        // Les entités 0 et 1 sont remplies de zéros pour forcer la recherche de pivot
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < NC; j++) {
                M[i][j] = 0.0;
            }
        }
    } else {
        // Les autres entités ont des nombres aléatoires entre 0 et 10
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < NC; j++) {
                M[i][j] = rand() % 11;
            }
        }
    }
    // Affichage de la matrice
    for (int i = 0; i < k; i++) {
        printf("[ ");
        for (int j = 0; j < NC; j++) {
            printf("%.0f ", M[i][j]);
        }
        printf("]\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Définition des balises pour éviter que les messages ne se croisent
    int TAG_RELAIS  = 10; // Message 0 ou 1 pour continuer/arrêter la recherche
    int TAG_LIGNE   = 11; // Envoi de la ligne du pivot trouvée
    int TAG_ECHANGE = 12; // Renvoi de l'ancienne ligne pour finaliser l'échange

    for (int l = 0; l < NL; l++) {
        int ligne_pivot[NC];    // Tampon pour diffuser le pivot final

        // PHASE 1 : RECHERCHE DU PIVOT ET ÉCHANGE
        if (id == l / k) {
            // L'entité l / k cherche d'abord dans ses propres lignes
            int pivot_trouve = 0;
            int m_local = -1;

            for (int m = l%k; m < k; m++) {  // l%k : indice local de cette ligne dans la mémoire de l / k -> l - k*i = l%k
                if (fabs(M[m][l]) != 0) {
                    pivot_trouve = 1;
                    m_local = m;
                    break;
                }
            }

            if (pivot_trouve) {
                // Elle l'a trouvé ! Elle fait l'échange localement
                if (m_local != l%k) {
                    for (int j = 0; j < NC; j++) {
                        int temp = M[l%k][j];
                        M[l%k][j] = M[m_local][j];
                        M[m_local][j] = temp;
                    }
                }
                // Et elle prévient les suivants (s'il y en a) que ce n'est plus la peine de chercher
                if (id + 1 < nb_processus) {
                    int msg = 0;
                    MPI_Send(&msg, 1, MPI_INT, id + 1, TAG_RELAIS, MPI_COMM_WORLD);
                }
            } else {
                // Elle ne l'a pas trouvé. Elle lance la recherche aux entités suivantes
                if (id + 1 < nb_processus) {
                    int msg = 1;
                    MPI_Send(&msg, 1, MPI_INT, id + 1, TAG_RELAIS, MPI_COMM_WORLD);
                }

                // Elle attend de recevoir la ligne de celui qui l'a trouvée
                int ligne_recue[NC];
                MPI_Recv(ligne_recue, NC, MPI_INT, MPI_ANY_SOURCE, TAG_LIGNE, MPI_COMM_WORLD, &status);

                // Elle renvoie son ancienne ligne (pleine de 0) à l'expéditeur pour finir l'échange
                MPI_Send(M[l%k], NC, MPI_INT, status.MPI_SOURCE, TAG_ECHANGE, MPI_COMM_WORLD);

                // Elle remplace sa ligne localement
                for (int j = 0; j < NC; j++) M[l%k][j] = ligne_recue[j];
            }

            // l / k a maintenant le bon pivot. Elle le divise par le pivot (normalisatilon)
            int pivot_val = M[l%k][l];
            for (int j = l; j < NC; j++) {
                M[l%k][j] = M[l%k][j] / pivot_val;
            }
            // On copie la ligne dans le tampon pour la diffusion
            for (int j = 0; j < NC; j++) ligne_pivot[j] = M[l%k][j];

        } 
        else if (id > l / k) {
            // Les entités en dessous de l'entité l / k (= l'entité posédant la ligne du pivot)
            int msg;
            MPI_Recv(&msg, 1, MPI_INT, id - 1, TAG_RELAIS, MPI_COMM_WORLD, &status);

            if (msg == 1) { // il faut chercher
                int pivot_trouve = 0;
                int m_local = -1;
                for (int m = 0; m < k; m++) {
                    if (fabs(M[m][l]) != 0) {
                        pivot_trouve = 1;
                        m_local = m;
                        break;
                    }
                }

                if (pivot_trouve) {
                    // On dit aux suivants d'arrêter de chercher
                    if (id + 1 < nb_processus) {
                        int msg = 0;
                        MPI_Send(&msg, 1, MPI_INT, id + 1, TAG_RELAIS, MPI_COMM_WORLD);
                    }
                    // On envoie notre ligne à l / k 
                    MPI_Send(M[m_local], NC, MPI_INT, l / k, TAG_LIGNE, MPI_COMM_WORLD);

                    // On reçoit l'ancienne ligne de l / k pour finaliser l'échange
                    MPI_Recv(M[m_local], NC, MPI_INT, l / k, TAG_ECHANGE, MPI_COMM_WORLD, &status);
                } else {
                    // On ne l'a pas trouvé non plus
                    if (id + 1 < nb_processus) {
                        int msg_out = 1;
                        MPI_Send(&msg_out, 1, MPI_INT, id + 1, TAG_RELAIS, MPI_COMM_WORLD);
                    }
                }
            } else { // msg == 0 (pivot déjà trouvé au-dessus)
                // On fait juste passer le message aux suivants
                if (id + 1 < nb_processus) {
                    int msg_out = 0;
                    MPI_Send(&msg_out, 1, MPI_INT, id + 1, TAG_RELAIS, MPI_COMM_WORLD);
                }
            }
        }

        // PHASE 2 : DIFFUSION (BCAST) DU PIVOT
        // Maintenant, l / k a la ligne parfaite. Il l'envoie à tout le monde d'un coup !
        MPI_Bcast(ligne_pivot, NC, MPI_INT, l / k, MPI_COMM_WORLD);

        // PHASE 3 : ÉLIMINATION 
        // Chaque entité utilise la ligne reçue pour mettre des zéros sous le pivot
        for (int m = 0; m < k; m++) {
            int global_m = id * k + m;
            if (global_m > l) { // On ne touche qu'aux lignes situées EN DESSOUS du pivot
                int facteur = M[m][l];
                for (int j = l; j < NC; j++) {
                    M[m][j] -= facteur * ligne_pivot[j];
                }
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Affichage de contrôle par le processus 0 (ou tous si on veut)
    if (id == nb_processus - 1) {
        printf("L'entité %d a terminé. La dernière ligne de sa matrice (x_n) est :\n", id);
        for(int j=0; j < NC; j++) printf("%.2f ", M[k-1][j]);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}