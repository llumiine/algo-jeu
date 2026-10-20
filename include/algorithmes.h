#ifndef ALGORITHMES_H
#define ALGORITHMES_H

#include <stddef.h>
#include "common.h"

/*
 * Interface commune a tous les algorithmes, utilisee par les benchmarks
 * (et utilisable par les tests). Chaque algorithme fournit les memes
 * operations ; "structure" est sa structure de donnees (grille, arbre...).
 *
 * Pour ajouter un algorithme (ex : quadtree) : ecrire ses fonctions
 * d'adaptation dans algorithmes.c et l'ajouter au tableau ALGORITHMES.
 */
typedef struct {
    const char* nom;

    void*  (*construire)(Monde* monde);
    void   (*mettre_a_jour)(void* structure);   /* apres deplacer_objets */
    void   (*liberer)(void* structure);
    size_t (*memoire)(void* structure);         /* en octets, sans le monde */

    Paire* (*collisions)(void* structure, int* nb);
    int*   (*objets_dans_boite)(void* structure, Point min, Point max, int* nb);
    int*   (*objets_dans_cercle)(void* structure, Point centre, float rayon, int* nb);
    int*   (*k_plus_proches)(void* structure, Point p, int k);
} Algorithme;

extern const Algorithme ALGORITHMES[];
extern const int NB_ALGORITHMES;

/* Renvoie l'algorithme de ce nom, ou NULL */
const Algorithme* trouver_algorithme(const char* nom);

#endif
