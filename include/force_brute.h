#ifndef FORCE_BRUTE_H
#define FORCE_BRUTE_H

#include "common.h"

Paire* fb_collisions(Monde* monde, int* nb);

int* fb_objets_dans_boite(Monde* monde, Point min, Point max, int* nb);

int* fb_objets_dans_cercle(Monde* monde, Point centre, float rayon, int* nb);

int* fb_k_plus_proches(Monde* monde, Point p, int k);

#endif