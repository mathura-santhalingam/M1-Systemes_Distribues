# TP1

Pour compiler et exécuter le programme, lancer les commandes

```bash
mpicc -Wall src.c -o nom_executable
mpirun -np N ./nom_executable
```
*Remarques : Pour exo6 et exo5_bis, il faut ajouter le nombre de tours en argument : mpirun -np 2 ./exo6 5*

## Contenu du Répertoire
- `exo1_2_3.c` : Découverte des bases. Initialisation de l'environnement (MPI_Init), récupération du nombre total de processus (MPI_Comm_size), de l'identifiant local (MPI_Comm_rank), et mesure du temps d'exécution (MPI_Wtime).
- `exemple_basique.c` : Algorithme basique du Ping-Pong (alternance des rôles avec pair/impair).
- `exo4.c` : Échange croisé d'identifiants entre deux processus.
- `exo5.c` : Échange croisé de variables distinctes (k et m). Le code est asymétrique (P0 envoie puis reçoit, P1 reçoit puis envoie) pour éviter l'interblocage (deadlock).
- `exo5_bis.c` : Version itérative de l'échange croisé (exo5.c). Les deux processus s'échangent leurs variables (m pour P0 et k pour P1) au sein d'une boucle sur $N$ tours.
- `exo6.c` : Véritable Ping-Pong itératif. Une valeur unique (la balle) passe de P0 à P1 et inversement sur $N$ tours. Le paramètre $N$ est lu depuis les arguments de la ligne de commande (argv[1]).
- `exo7.c` : Topologie en Anneau (Ring). Un entier est relayé en cascade circulaire : $P_0$ l'envoie à $P_1$, qui l'envoie à $P_2$... jusqu'au dernier processus $P_{n-1}$ qui le renvoie à $P_0$.
- `exo8_v1.c` : Calcul Min/Max en Pipeline (Bloc contigu). Propagation de proche en proche des valeurs extrêmes. Optimisation des envois en transmettant un tableau regroupant les deux variables [min, max].
- `exo8_v2.c` : Calcul Min/Max en Pipeline (Discrimination par Balises). Envoi des valeurs séparément. L'ordre de réception n'a pas d'importance car les messages sont différenciés par des tags MPI uniques (Balise 0 pour le Min, Balise 1 pour le Max).
- `exo8_v3.c` : Calcul Min/Max en Pipeline (Garantie FIFO). Envoi des valeurs séparément avec la même balise. Fonctionne grâce à la règle stricte de séquencement de MPI (les messages entre une même source et destination arrivent toujours dans l'ordre de leur émission).