#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// Envoyer le max

int main(int argc, char** argv) {
    int n, id;
    int p; // Ton maximum à toi
    int q; // La boîte de réception pour les messages des voisins
    int k, voisin_gauche, voisin_droite;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Tirer au hasard un nombre p entre 0 et 99 <= CHAQUE ENTITÉ LE FAIT => pas dans une boucle if(id == 0)
    srand(time(NULL) + id); 
    p = rand() % 100; // Initialisation de ton max
    
    k = n / 2; 
    voisin_gauche = (n + id - 1) % n;
    voisin_droite = (id + 1) % n;
    
    // Pour j allant de 0 à k faire
    for (int j = 0; j <= k; j++) {
        
        // ==========================================
        // 1. Échange avec le voisin de GAUCHE 
        // ==========================================
        if (id % 2 == 0) {
            MPI_Send(&p, 1, MPI_INT, voisin_droite, 0, MPI_COMM_WORLD);
            MPI_Recv(&q, 1, MPI_INT, voisin_gauche, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(&q, 1, MPI_INT, voisin_gauche, 0, MPI_COMM_WORLD, &status);
            MPI_Send(&p, 1, MPI_INT, voisin_droite, 0, MPI_COMM_WORLD);
        }
        
        // Poser p <- max{p, q}
        if (q > p) p = q;

        // ==========================================
        // 2. Échange avec le voisin de DROITE
        // ==========================================
        if (id % 2 == 0) {
            MPI_Send(&p, 1, MPI_INT, voisin_gauche, 1, MPI_COMM_WORLD);
            MPI_Recv(&q, 1, MPI_INT, voisin_droite, 1, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(&q, 1, MPI_INT, voisin_droite, 1, MPI_COMM_WORLD, &status);
            MPI_Send(&p, 1, MPI_INT, voisin_gauche, 1, MPI_COMM_WORLD);
        }
        
        // Poser p <- max{p, q}
        if (q > p) p = q;
    }

    printf("Processus %d affiche le maximum : %d\n", id, p);

    MPI_Finalize();
    return 0;
}