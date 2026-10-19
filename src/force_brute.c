#include <stdlib.h>
#include "force_brute.h"


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


Paire* fb_collisions(Monde* monde, int* nb) {
    Paire* resultat = NULL;
    int taille = 0;
    int capacite = 0;

    for (int i = 0; i < monde->nb_objets; i++) {
        Objet* o1 = monde->objets + i;

       
        for (int j = i + 1; j < monde->nb_objets; j++) {
            Objet* o2 = monde->objets + j;

            float somme_rayons = o1->rayon + o2->rayon;
            if (distance_carre(o1->position, o2->position) <= somme_rayons * somme_rayons) {
                ajouter_paire(&resultat, &taille, &capacite, i, j);
            }
        }
    }

    *nb = taille;
    return resultat;
}

int* fb_objets_dans_boite(Monde* monde, Point min, Point max, int* nb) {
    int* resultat = NULL;
    int taille = 0;
    int capacite = 0;

    for (int i = 0; i < monde->nb_objets; i++) {
        Objet* o = monde->objets + i;

        
        Point proche;
        proche.x = bloquer(o->position.x, min.x, max.x);
        proche.y = bloquer(o->position.y, min.y, max.y);
        proche.z = bloquer(o->position.z, min.z, max.z);

        if (distance_carre(o->position, proche) <= o->rayon * o->rayon) {
            ajouter_entier(&resultat, &taille, &capacite, o->id);
        }
    }

    *nb = taille;
    return resultat;
}


int* fb_objets_dans_cercle(Monde* monde, Point centre, float rayon, int* nb) {
    int* resultat = NULL;
    int taille = 0;
    int capacite = 0;

    for (int i = 0; i < monde->nb_objets; i++) {
        Objet* o = monde->objets + i;
        float somme_rayons = o->rayon + rayon;

        if (distance_carre(o->position, centre) <= somme_rayons * somme_rayons) {
            ajouter_entier(&resultat, &taille, &capacite, o->id);
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

int* fb_k_plus_proches(Monde* monde, Point p, int k) {
    int n = monde->nb_objets;
    if (k > n) k = n;       
    if (k <= 0) return NULL;

    Candidat* candidats = malloc(n * sizeof(Candidat));
    if (candidats == NULL) return NULL;
    for (int i = 0; i < n; i++) {
        (candidats + i)->id = i;
        (candidats + i)->distance = distance_carre((monde->objets + i)->position, p);
    }

  
    qsort(candidats, n, sizeof(Candidat), comparer_candidats);

    int* resultat = malloc(k * sizeof(int));
    if (resultat != NULL) {
        for (int i = 0; i < k; i++) {
            *(resultat + i) = (candidats + i)->id;
        }
    }

    free(candidats);
    return resultat;
}