#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#define t 12

/* IMPLÉMENTATION DE :
    Admettons que chaque entité de calcul a un tableau M de k cases.
    ÉTAPE 1 : Les deux premieres entités sont remplies de 0 et les autres ont des valeurs aléatoires.
    ÉTAPE 2 : chaque entité lit son tableau, si elle trouve que des 0 elle ne fait rien.
    On veut trouver, parmi les tableaux, le premier tableau qui a une valeur non nulle.
    Donc le id le plus petit tel que le M de ce id contient une valeur non nulle.
    Ensuite : une fois trouvé cet id, échange sa 1ère valeur non nulle avec la valeur 0 de l’entité 0 à la même position.
    
    Attention : utilise Bcast(buffer, k, MPI_Type, source, MPI_COMM_WORLD);
    Attention : le buffer de reception ne doit pas ecraser directement la valeur dans le tableau car par ex un bcast d'une entité de grand id peut écraser la valeur qu'on cherchait 

*/

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
  
  printf("Je suis la machine %d et la machine d'entité %d est le pivot\n", i, buff);
  
  // ÉCHANGE FINAL
  // buff contient l'id du pivot. Si buff == n, c'est que personne n'a rien trouvé.
  if (buff != n && buff != 0) {
      int temp[k]; // Un tableau temporaire pour la réception
      
      if (i == 0) {
          // L'entité 0 ENVOIE d'abord, puis REÇOIT
          MPI_Send(M, k, MPI_INT, buff, 0, MPI_COMM_WORLD);
          MPI_Recv(temp, k, MPI_INT, buff, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          
          for (int m = 0; m < k; m++) M[m] = temp[m];
          
      } else if (i == buff) {
          // L'entité pivot REÇOIT d'abord, puis ENVOIE
          MPI_Recv(temp, k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(M, k, MPI_INT, 0, 0, MPI_COMM_WORLD);
          
          for (int m = 0; m < k; m++) M[m] = temp[m];
          
          // Et voici l'utilisation de index_trouve qui va supprimer le warning !
          printf("-> Succès ! Je suis %d, j'ai échangé ma ligne avec 0. Mon pivot était à l'indice %d.\n", i, index_trouve);
      }
  }
  MPI_Finalize();
}