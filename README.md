# Systèmes Distribués

Ce répertoire rassemble l'ensemble de mes notes de cours et travaux pratiques réalisés dans le cadre du module **Systèmes Distribués**, enseigné par Lionel Pournin à l'Université Sorbonne Paris Nord. Les notes de cours sont en partie inspirées des travaux de Cyril Gavoille (Université de Bordeaux).

---

## Contenu du cours

### Thèmes abordés

- **Modèle de communication par passage de messages** — pseudo-code, traces d'exécution, modèles LOCAL et CONGEST
- **Problème de la diffusion** — algorithmes FLOOD, CAST, FLOOD&ECHO
- **Construction d'arbres couvrants** — arbres de parcours en largeur, algorithme de Bellman-Ford distribué, algorithme de Dijkstra distribué
- **Problèmes de réduction et concentration** — algorithmes REDUC et CONC
- **Communicateurs MPI** — `MPI_Comm_split`, topologies cartésiennes (`MPI_Cart_create`, `MPI_Cart_shift`, `MPI_Cart_sub`)

### Algorithmes implémentés (C + MPI)

| Algorithme | Fichier(s) |
|---|---|
| CAST (tas binaire) | `TP2_SD/` |
| FLOOD (graphe complet) | `TP2_SD/` |
| FLOOD&ECHO | `TP3_SD/` |
| CAST sur arbre FLOOD&ECHO | `TP3_SD/` |
| CONC | `TP3_SD/` |
| Bellman-Ford distribué | `TP4_SD/` |
| Dijkstra distribué | `TP4_SD/` |
| Communicateurs & topologie cartésienne | `TP_Comm/` |
| Gauss-Jordan distribué | `TP_Gauss/` |

---

### Compilation

```bash
mpicc -Wall mon_programme.c -o mon_programme
```

### Exécution

```bash
mpirun -np <nombre_de_processus> ./mon_programme
```

---

## Structure du dépôt

```
.
├── Notes_De_Cours.pdf  # Mes prises de notes personnelles du CM
├── Sujets_SD/          # Sujets de TDs, TPs et examens (PDF)
│   ├── TD1.pdf         # Modèle de communication par passage de messages
│   ├── TD2.pdf         # Problème de la diffusion
│   ├── TD3.pdf         # Construction d'arbres
│   ├── TD4.pdf         # Communicateurs
│   ├── TP1.pdf         # Passage de messages avec MPI
│   ├── TP2.pdf         # Diffusion (première partie)
│   ├── TP3.pdf         # Diffusion (deuxième partie)
│   ├── TP4.pdf         # Bellman-Ford et Dijkstra distribué
│   ├── TP5.pdf         # Communicateurs MPI
│   ├── SD_CM.pdf       # Notes de cours de Mr.Pournin
│   ├── ExamenBlanc.pdf         # Examen blanc (avril 2021)
│   └── Examen_P2_blanc.pdf     # Examen blanc (partie 2)
└── TP_SD/              # Implémentations C avec MPI
    ├── TP1_SD/         # Exercices TP1
    ├── TP2_SD/         # Exercices TP2
    ├── TP3_SD/         # Exercices TP3
    ├── TP4_SD/         # Exercices TP4
    ├── TP5_SD/         # Exercices TP5
    ├── Exams/          # Corrections d'examens
    ├── TP_Comm/        # TP Communicateurs (bcast, gather, reduce, split, topo...)
    └── TP_Gauss/       # TP Gauss-Jordan distribué
```


## Références

- **Lionel Pournin**, *SD_CM* (dans le dossier **Sujets_SD**) — Université Sorbonne Paris Nord
- **Cyril Gavoille**, *Algorithmes distribués*, Cours de Master 1 et 2, Université de Bordeaux, 2015