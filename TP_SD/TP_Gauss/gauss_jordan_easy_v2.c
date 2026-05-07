#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#define t 12

/*
   Compilation : mpicc -Wall gauss_jordan_easy_v2.c -o gauss_jordan_easy_v2.exe
   Exécution   : mpirun -np 7 ./gauss_jordan_easy_v2.exe
*/

int main(){
  int i, n; 
  MPI_Init(NULL,NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  MPI_Comm_rank(MPI_COMM_WORLD, &i);
  
  int k=t/n;
  int M[k];
  
  srand(time(NULL)+i);
  
  if(i<2){
    for(int m=0; m<k; m++){
      M[m]=0;
    }
  }
  else{
    for(int m=0; m<k; m++){
      M[m]=rand()%10;
    }
  }
  
  int buff;
  int val=n;
  int r=0;
  
  int val = n;
    int index_trouve = -1; // Pour mémoriser la position

    for (int r = 0; r < k; r++) {
        if (M[r] != 0) {
            val = i;
            index_trouve = r; // On sauvegarde la position !
            break; // On a trouvé le 1er non-zéro, on stoppe la recherche
        }
    }
  
  MPI_Reduce(&val, &buff, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
  
  MPI_Bcast(&buff, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  printf("Je suis la machine %d et la machine d'entitÃ© %d est le pivot\n", i, buff);
  
  MPI_Finalize();
}