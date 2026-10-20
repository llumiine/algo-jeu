#include <stdlib.h>
#include <string.h>
#include "algorithmes.h"
#include "force_brute.h"
#include "grille.h"

/* ---------- Force brute : pas de structure, on garde juste le monde ---------- */

static void* fb_construire(Monde* monde) { return monde; }
static void fb_mettre_a_jour(void* s) { (void) s; }
static void fb_liberer(void* s) { (void) s; }
static size_t fb_memoire(void* s) { (void) s; return 0; }

static Paire* fb_adapt_collisions(void* s, int* nb) {
    return fb_collisions((Monde*) s, nb);
}
static int* fb_adapt_boite(void* s, Point min, Point max, int* nb) {
    return fb_objets_dans_boite((Monde*) s, min, max, nb);
}
static int* fb_adapt_cercle(void* s, Point centre, float rayon, int* nb) {
    return fb_objets_dans_cercle((Monde*) s, centre, rayon, nb);
}
static int* fb_adapt_kppv(void* s, Point p, int k) {
    return fb_k_plus_proches((Monde*) s, p, k);
}

/* ---------- Grille uniforme ---------- */

static void* grille_adapt_construire(Monde* monde) {
    Grille* grille = malloc(sizeof(Grille));
    if (grille == NULL) return NULL;
    *grille = grille_construire(monde, 0);   /* taille de cellule automatique */
    return grille;
}
static void grille_adapt_mettre_a_jour(void* s) { grille_mettre_a_jour((Grille*) s); }
static void grille_adapt_liberer(void* s) {
    grille_liberer((Grille*) s);
    free(s);
}
static size_t grille_adapt_memoire(void* s) { return grille_memoire((Grille*) s); }

static Paire* grille_adapt_collisions(void* s, int* nb) {
    return grille_collisions((Grille*) s, nb);
}
static int* grille_adapt_boite(void* s, Point min, Point max, int* nb) {
    return grille_objets_dans_boite((Grille*) s, min, max, nb);
}
static int* grille_adapt_cercle(void* s, Point centre, float rayon, int* nb) {
    return grille_objets_dans_cercle((Grille*) s, centre, rayon, nb);
}
static int* grille_adapt_kppv(void* s, Point p, int k) {
    return grille_k_plus_proches((Grille*) s, p, k);
}


const Algorithme ALGORITHMES[] = {
    { "force_brute", fb_construire, fb_mettre_a_jour, fb_liberer, fb_memoire,
      fb_adapt_collisions, fb_adapt_boite, fb_adapt_cercle, fb_adapt_kppv },

    { "grille", grille_adapt_construire, grille_adapt_mettre_a_jour, grille_adapt_liberer, grille_adapt_memoire,
      grille_adapt_collisions, grille_adapt_boite, grille_adapt_cercle, grille_adapt_kppv },

    /* TODO personne C : { "quadtree", ... }, puis { "octree", ... } */
};

const int NB_ALGORITHMES = sizeof(ALGORITHMES) / sizeof(ALGORITHMES[0]);

const Algorithme* trouver_algorithme(const char* nom) {
    for (int i = 0; i < NB_ALGORITHMES; i++) {
        if (strcmp((ALGORITHMES + i)->nom, nom) == 0) return ALGORITHMES + i;
    }
    return NULL;
}
