#ifndef MONDE_H
#define MONDE_H

#include "common.h"

Monde creer_monde_uniforme(int nb, float taille, float rayon, int en_3d);

Monde creer_monde_amas(int nb, float taille, float rayon, int nb_amas, int en_3d);

void deplacer_objets(Monde* monde, float dt);

void liberer_monde(Monde* monde);

#endif