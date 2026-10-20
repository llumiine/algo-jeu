#include <stdlib.h>
#include <math.h>
#include "grille.h"

/* Nombre maximum de cellules quand la taille est choisie automatiquement :
   au-dela, la grille coute plus en memoire qu'elle ne fait gagner. */
#define CELLULES_PAR_OBJET 8
#define CELLULES_MAX (1 << 26)

/* Plage de cellules [x0, x1] x [y0, y1] x [z0, z1] (bornes incluses) */
typedef struct {
    int x0, x1;
    int y0, y1;
    int z0, z1;
} Plage;

static void ajouter_entier(int** tab, int* taille, int* capacite, int valeur) {
    if (*taille == *capacite) {
        int nouvelle_capacite = (*capacite == 0) ? 16 : *capacite * 2;
        int* nouveau = realloc(*tab, nouvelle_capacite * sizeof(int));
        if (nouveau == NULL) return;
        *tab = nouveau;
        *capacite = nouvelle_capacite;
    }
    *(*tab + *taille) = valeur;
    *taille = *taille + 1;
}

static void ajouter_paire(Paire** tab, int* taille, int* capacite, int a, int b) {
    if (*taille == *capacite) {
        int nouvelle_capacite = (*capacite == 0) ? 16 : *capacite * 2;
        Paire* nouveau = realloc(*tab, nouvelle_capacite * sizeof(Paire));
        if (nouveau == NULL) return;
        *tab = nouveau;
        *capacite = nouvelle_capacite;
    }
    (*tab + *taille)->a = a;
    (*tab + *taille)->b = b;
    *taille = *taille + 1;
}

static float bloquer(float valeur, float min, float max) {
    if (valeur < min) return min;
    if (valeur > max) return max;
    return valeur;
}

/* Numero de cellule sur un axe. Les objets sortis du monde sont ranges
   dans la cellule du bord (deplacer_objets peut les faire depasser un peu). */
static int indice_axe(float valeur, float taille_cellule, int n) {
    float i = floorf(valeur / taille_cellule);
    if (!(i >= 0)) return 0;   /* gere aussi NaN */
    if (i >= n) return n - 1;
    return (int) i;
}

static int nb_cellules_axe(float longueur, float taille_cellule) {
    double n = ceil(longueur / taille_cellule);
    if (n < 1) return 1;
    if (n > CELLULES_MAX) return CELLULES_MAX;
    return (int) n;
}

static int cellule_du_point(Grille* grille, Point p) {
    int x = indice_axe(p.x, grille->taille_cellule, grille->nx);
    int y = indice_axe(p.y, grille->taille_cellule, grille->ny);
    int z = indice_axe(p.z, grille->taille_cellule, grille->nz);
    return (z * grille->ny + y) * grille->nx + x;
}

/* Cellules qui touchent la boite [min, max] */
static Plage plage_cellules(Grille* grille, Point min, Point max) {
    Plage plage;
    plage.x0 = indice_axe(min.x, grille->taille_cellule, grille->nx);
    plage.x1 = indice_axe(max.x, grille->taille_cellule, grille->nx);
    plage.y0 = indice_axe(min.y, grille->taille_cellule, grille->ny);
    plage.y1 = indice_axe(max.y, grille->taille_cellule, grille->ny);
    plage.z0 = indice_axe(min.z, grille->taille_cellule, grille->nz);
    plage.z1 = indice_axe(max.z, grille->taille_cellule, grille->nz);
    return plage;
}

/* Cellules qui touchent le cube de demi-cote "marge" autour de p */
static Plage plage_autour(Grille* grille, Point p, float marge) {
    Point min = { p.x - marge, p.y - marge, p.z - marge };
    Point max = { p.x + marge, p.y + marge, p.z + marge };
    return plage_cellules(grille, min, max);
}

static float calculer_rayon_max(Monde* monde) {
    float rayon_max = 0;
    for (int i = 0; i < monde->nb_objets; i++) {
        if ((monde->objets + i)->rayon > rayon_max) {
            rayon_max = (monde->objets + i)->rayon;
        }
    }
    return rayon_max;
}

/* Calcule nx, ny, nz et nb_cellules pour la taille de cellule actuelle */
static double dimensionner(Grille* grille) {
    Monde* monde = grille->monde;
    grille->nx = nb_cellules_axe(monde->largeur, grille->taille_cellule);
    grille->ny = nb_cellules_axe(monde->hauteur, grille->taille_cellule);
    grille->nz = (monde->profondeur > 0) ? nb_cellules_axe(monde->profondeur, grille->taille_cellule) : 1;
    return (double) grille->nx * grille->ny * grille->nz;
}

/* Range chaque objet dans sa cellule (tri par comptage, O(n + nb_cellules)) */
static void remplir(Grille* grille) {
    Monde* monde = grille->monde;
    int* debut = grille->debut;

    for (int c = 0; c <= grille->nb_cellules; c++) {
        *(debut + c) = 0;
    }

    /* 1. compter les objets de chaque cellule (dans la case c + 1) */
    for (int i = 0; i < grille->nb_objets; i++) {
        int c = cellule_du_point(grille, (monde->objets + i)->position);
        *(grille->cellule_de + i) = c;
        *(debut + c + 1) = *(debut + c + 1) + 1;
    }

    /* 2. sommes cumulees : debut[c] = position de la premiere case de c */
    for (int c = 0; c < grille->nb_cellules; c++) {
        *(debut + c + 1) = *(debut + c + 1) + *(debut + c);
    }

    /* 3. placer les objets ; debut[c] avance jusqu'au debut de la cellule suivante */
    for (int i = 0; i < grille->nb_objets; i++) {
        int c = *(grille->cellule_de + i);
        *(grille->indices + *(debut + c)) = i;
        *(debut + c) = *(debut + c) + 1;
    }

    /* 4. decaler d'une case pour retrouver les debuts */
    for (int c = grille->nb_cellules; c > 0; c--) {
        *(debut + c) = *(debut + c - 1);
    }
    *debut = 0;
}


Grille grille_construire(Monde* monde, float taille_cellule) {
    Grille grille = {0};
    grille.monde = monde;
    grille.nb_objets = monde->nb_objets;
    grille.rayon_max = calculer_rayon_max(monde);

    int automatique = (taille_cellule <= 0);
    if (automatique) {
        taille_cellule = 2 * grille.rayon_max;
        if (taille_cellule <= 0) taille_cellule = monde->largeur / 16;
        if (taille_cellule <= 0) taille_cellule = 1;
    }
    grille.taille_cellule = taille_cellule;

    double limite = automatique ? (double) CELLULES_PAR_OBJET * monde->nb_objets + 64 : CELLULES_MAX;
    if (limite > CELLULES_MAX) limite = CELLULES_MAX;
    double nb_cellules = dimensionner(&grille);
    while (nb_cellules > limite) {
        grille.taille_cellule = grille.taille_cellule * 1.25f;
        nb_cellules = dimensionner(&grille);
    }
    grille.nb_cellules = (int) nb_cellules;

    grille.debut = malloc((grille.nb_cellules + 1) * sizeof(int));
    grille.indices = malloc((grille.nb_objets + 1) * sizeof(int));
    grille.cellule_de = malloc((grille.nb_objets + 1) * sizeof(int));
    if (grille.debut == NULL || grille.indices == NULL || grille.cellule_de == NULL) {
        grille_liberer(&grille);
        return grille;
    }

    remplir(&grille);
    return grille;
}

void grille_mettre_a_jour(Grille* grille) {
    if (grille->monde->nb_objets != grille->nb_objets) {
        /* le nombre d'objets a change : on reconstruit avec la meme taille de cellule */
        Monde* monde = grille->monde;
        float taille_cellule = grille->taille_cellule;
        grille_liberer(grille);
        *grille = grille_construire(monde, taille_cellule);
        return;
    }
    if (grille->debut == NULL) return;

    grille->rayon_max = calculer_rayon_max(grille->monde);
    remplir(grille);
}

void grille_liberer(Grille* grille) {
    free(grille->debut);
    free(grille->indices);
    free(grille->cellule_de);
    grille->debut = NULL;
    grille->indices = NULL;
    grille->cellule_de = NULL;
    grille->nb_cellules = 0;
    grille->nb_objets = 0;
}

size_t grille_memoire(Grille* grille) {
    if (grille->debut == NULL) return sizeof(Grille);
    return sizeof(Grille)
         + (size_t) (grille->nb_cellules + 1) * sizeof(int)
         + (size_t) 2 * (grille->nb_objets + 1) * sizeof(int);
}


Paire* grille_collisions(Grille* grille, int* nb) {
    Paire* resultat = NULL;
    int taille = 0;
    int capacite = 0;
    Monde* monde = grille->monde;

    if (grille->debut == NULL) {
        *nb = 0;
        return NULL;
    }

    for (int i = 0; i < grille->nb_objets; i++) {
        Objet* o1 = monde->objets + i;

        /* un objet j touche o1 seulement si son centre est a moins de
           o1->rayon + rayon_max : on ne regarde que ces cellules */
        Plage plage = plage_autour(grille, o1->position, o1->rayon + grille->rayon_max);

        for (int z = plage.z0; z <= plage.z1; z++) {
            for (int y = plage.y0; y <= plage.y1; y++) {
                for (int x = plage.x0; x <= plage.x1; x++) {
                    int c = (z * grille->ny + y) * grille->nx + x;

                    for (int k = *(grille->debut + c); k < *(grille->debut + c + 1); k++) {
                        int j = *(grille->indices + k);
                        if (j <= i) continue;   /* chaque paire une seule fois */

                        Objet* o2 = monde->objets + j;
                        float somme_rayons = o1->rayon + o2->rayon;
                        if (distance_carre(o1->position, o2->position) <= somme_rayons * somme_rayons) {
                            ajouter_paire(&resultat, &taille, &capacite, i, j);
                        }
                    }
                }
            }
        }
    }

    *nb = taille;
    return resultat;
}

int* grille_objets_dans_boite(Grille* grille, Point min, Point max, int* nb) {
    int* resultat = NULL;
    int taille = 0;
    int capacite = 0;
    Monde* monde = grille->monde;

    if (grille->debut == NULL) {
        *nb = 0;
        return NULL;
    }

    float r = grille->rayon_max;
    Point min_elargi = { min.x - r, min.y - r, min.z - r };
    Point max_elargi = { max.x + r, max.y + r, max.z + r };
    Plage plage = plage_cellules(grille, min_elargi, max_elargi);

    for (int z = plage.z0; z <= plage.z1; z++) {
        for (int y = plage.y0; y <= plage.y1; y++) {
            for (int x = plage.x0; x <= plage.x1; x++) {
                int c = (z * grille->ny + y) * grille->nx + x;

                for (int k = *(grille->debut + c); k < *(grille->debut + c + 1); k++) {
                    Objet* o = monde->objets + *(grille->indices + k);

                    /* meme test que la force brute : point de la boite le plus proche */
                    Point proche;
                    proche.x = bloquer(o->position.x, min.x, max.x);
                    proche.y = bloquer(o->position.y, min.y, max.y);
                    proche.z = bloquer(o->position.z, min.z, max.z);

                    if (distance_carre(o->position, proche) <= o->rayon * o->rayon) {
                        ajouter_entier(&resultat, &taille, &capacite, o->id);
                    }
                }
            }
        }
    }

    *nb = taille;
    return resultat;
}

int* grille_objets_dans_cercle(Grille* grille, Point centre, float rayon, int* nb) {
    int* resultat = NULL;
    int taille = 0;
    int capacite = 0;
    Monde* monde = grille->monde;

    if (grille->debut == NULL) {
        *nb = 0;
        return NULL;
    }

    Plage plage = plage_autour(grille, centre, rayon + grille->rayon_max);

    for (int z = plage.z0; z <= plage.z1; z++) {
        for (int y = plage.y0; y <= plage.y1; y++) {
            for (int x = plage.x0; x <= plage.x1; x++) {
                int c = (z * grille->ny + y) * grille->nx + x;

                for (int k = *(grille->debut + c); k < *(grille->debut + c + 1); k++) {
                    Objet* o = monde->objets + *(grille->indices + k);
                    float somme_rayons = o->rayon + rayon;

                    if (distance_carre(o->position, centre) <= somme_rayons * somme_rayons) {
                        ajouter_entier(&resultat, &taille, &capacite, o->id);
                    }
                }
            }
        }
    }

    *nb = taille;
    return resultat;
}

typedef struct {
    int   id;
    float distance;
} Candidat;

static int comparer_candidats(const void* a, const void* b) {
    const Candidat* c1 = a;
    const Candidat* c2 = b;
    if (c1->distance < c2->distance) return -1;
    if (c1->distance > c2->distance) return 1;
    return 0;
}

/*
 * On ramasse les objets des cellules autour de p dans un rayon r, puis on
 * double r tant qu'on n'est pas sur d'avoir les k plus proches.
 * Tout objet a distance <= r de p a son centre dans les cellules visitees :
 * si le k-ieme candidat est a distance <= r, aucun objet non visite ne peut
 * etre plus proche que lui.
 */
int* grille_k_plus_proches(Grille* grille, Point p, int k) {
    int n = grille->nb_objets;
    if (k > n) k = n;
    if (k <= 0 || grille->debut == NULL) return NULL;

    Candidat* candidats = malloc(n * sizeof(Candidat));
    if (candidats == NULL) return NULL;

    Monde* monde = grille->monde;
    float r = grille->taille_cellule;
    int nb_candidats = 0;

    while (1) {
        Plage plage = plage_autour(grille, p, r);
        int tout_vu = (plage.x0 == 0 && plage.x1 == grille->nx - 1 &&
                       plage.y0 == 0 && plage.y1 == grille->ny - 1 &&
                       plage.z0 == 0 && plage.z1 == grille->nz - 1);

        nb_candidats = 0;
        for (int z = plage.z0; z <= plage.z1; z++) {
            for (int y = plage.y0; y <= plage.y1; y++) {
                for (int x = plage.x0; x <= plage.x1; x++) {
                    int c = (z * grille->ny + y) * grille->nx + x;

                    for (int i = *(grille->debut + c); i < *(grille->debut + c + 1); i++) {
                        int j = *(grille->indices + i);
                        (candidats + nb_candidats)->id = j;
                        (candidats + nb_candidats)->distance = distance_carre((monde->objets + j)->position, p);
                        nb_candidats++;
                    }
                }
            }
        }

        if (nb_candidats >= k) {
            qsort(candidats, nb_candidats, sizeof(Candidat), comparer_candidats);
            if (tout_vu || (candidats + k - 1)->distance <= r * r) break;
        }
        r = r * 2;
    }

    int* resultat = malloc(k * sizeof(int));
    if (resultat != NULL) {
        for (int i = 0; i < k; i++) {
            *(resultat + i) = (candidats + i)->id;
        }
    }

    free(candidats);
    return resultat;
}
