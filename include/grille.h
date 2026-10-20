#ifndef GRILLE_H
#define GRILLE_H

#include <stddef.h>
#include "common.h"

/*
 * Grille uniforme (2D ou 3D).
 *
 * Le monde est decoupe en cellules cubiques de meme taille. Chaque objet est
 * range dans la cellule qui contient son centre. Les objets d'une meme cellule
 * sont stockes les uns a la suite des autres dans "indices" (tri par comptage) :
 * les objets de la cellule c sont indices[debut[c]] ... indices[debut[c + 1] - 1].
 *
 * Comme un objet n'est range que par son centre, une recherche doit regarder
 * les cellules a une distance "rayon_max" autour de la zone demandee.
 *
 * En 2D (profondeur du monde = 0), il n'y a qu'une seule couche de cellules (nz = 1).
 */
typedef struct {
    Monde* monde;          /* monde indexe par la grille */
    int    nb_objets;      /* nombre d'objets au moment de la construction */
    float  taille_cellule; /* cote d'une cellule */
    int    nx, ny, nz;     /* nombre de cellules sur chaque axe */
    int    nb_cellules;    /* nx * ny * nz */
    float  rayon_max;      /* plus grand rayon d'objet */
    int*   debut;          /* nb_cellules + 1 cases : debut de chaque cellule dans indices */
    int*   indices;        /* nb_objets cases : indices des objets tries par cellule */
    int*   cellule_de;     /* nb_objets cases : cellule de chaque objet */
} Grille;

/* Construit la grille. Si taille_cellule <= 0, elle est choisie automatiquement
   (2 * rayon_max, agrandie si le nombre de cellules devient trop grand). */
Grille grille_construire(Monde* monde, float taille_cellule);

/* Remet a jour la grille apres un deplacement des objets (reutilise la memoire). */
void grille_mettre_a_jour(Grille* grille);

void grille_liberer(Grille* grille);

/* Memoire occupee par la grille, en octets (le monde n'est pas compte). */
size_t grille_memoire(Grille* grille);

Paire* grille_collisions(Grille* grille, int* nb);

int* grille_objets_dans_boite(Grille* grille, Point min, Point max, int* nb);

int* grille_objets_dans_cercle(Grille* grille, Point centre, float rayon, int* nb);

int* grille_k_plus_proches(Grille* grille, Point p, int k);

#endif
