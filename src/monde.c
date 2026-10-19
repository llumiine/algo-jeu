#include <stdlib.h>
#include "monde.h"

/* Distance au carré entre deux points */
float distance_carre(Point p1, Point p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float dz = p1.z - p2.z;
    return dx * dx + dy * dy + dz * dz;
}

/* Renvoie un nombre au hasard entre min et max */
static float hasard(float min, float max) {
    return min + (max - min) * ((float) rand() / (float) RAND_MAX);
}

/* Garde une valeur entre 0 et max (pour ne pas sortir du monde) */
static float limiter(float valeur, float max) {
    if (valeur < 0) return 0;
    if (valeur > max) return max;
    return valeur;
}

/* Donne une vitesse au hasard à un objet */
static void vitesse_hasard(Objet* objet, int en_3d) {
    objet->vitesse.x = hasard(-1, 1);
    objet->vitesse.y = hasard(-1, 1);
    if (en_3d) {
        objet->vitesse.z = hasard(-1, 1);
    } else {
        objet->vitesse.z = 0;
    }
}

/* creation du monde */
Monde creer_monde_uniforme(int nb, float taille, float rayon, int en_3d) {
    Monde monde;
    monde.nb_objets = nb;
    monde.largeur = taille;
    monde.hauteur = taille;
    monde.profondeur = en_3d ? taille : 0;

    monde.objets = malloc(nb * sizeof(Objet));
    if (monde.objets == NULL) {
        monde.nb_objets = 0;
        return monde;
    }

    for (int i = 0; i < nb; i++) {
        Objet* objet = monde.objets + i;
        objet->id = i;
        objet->rayon = rayon;
        objet->position.x = hasard(0, taille);
        objet->position.y = hasard(0, taille);
        objet->position.z = en_3d ? hasard(0, taille) : 0;
        vitesse_hasard(objet, en_3d);
    }
    return monde;
}

Monde creer_monde_amas(int nb, float taille, float rayon, int nb_amas, int en_3d) {
    Monde monde;
    monde.nb_objets = nb;
    monde.largeur = taille;
    monde.hauteur = taille;
    monde.profondeur = en_3d ? taille : 0;

    monde.objets = malloc(nb * sizeof(Objet));
    Point* centres = malloc(nb_amas * sizeof(Point));
    if (monde.objets == NULL || centres == NULL || nb_amas < 1) {
        free(monde.objets);
        free(centres);
        monde.objets = NULL;
        monde.nb_objets = 0;
        return monde;
    }

    /* choisit au hasrad le centre */
    for (int c = 0; c < nb_amas; c++) {
        (centres + c)->x = hasard(0, taille);
        (centres + c)->y = hasard(0, taille);
        (centres + c)->z = en_3d ? hasard(0, taille) : 0;
    }

    /* place les objets autour */
    float ecart = taille / 20;   /* taille d'un amas */
    for (int i = 0; i < nb; i++) {
        Objet* objet = monde.objets + i;
        Point* centre = centres + (i % nb_amas);   /* on répartit les objets */

        objet->id = i;
        objet->rayon = rayon;
        objet->position.x = limiter(centre->x + hasard(-ecart, ecart), taille);
        objet->position.y = limiter(centre->y + hasard(-ecart, ecart), taille);
        objet->position.z = en_3d ? limiter(centre->z + hasard(-ecart, ecart), taille) : 0;
        vitesse_hasard(objet, en_3d);
    }

    free(centres);
    return monde;
}


void deplacer_objets(Monde* monde, float dt) {
    for (int i = 0; i < monde->nb_objets; i++) {
        Objet* objet = monde->objets + i;

        /*  position = position + vitesse * dt */
        objet->position.x = objet->position.x + objet->vitesse.x * dt;
        objet->position.y = objet->position.y + objet->vitesse.y * dt;
        objet->position.z = objet->position.z + objet->vitesse.z * dt;

        /* Si on sort du monde, on fait demi-tour (rebond) */
        if (objet->position.x < 0 || objet->position.x > monde->largeur) {
            objet->vitesse.x = -objet->vitesse.x;
        }
        if (objet->position.y < 0 || objet->position.y > monde->hauteur) {
            objet->vitesse.y = -objet->vitesse.y;
        }
        if (monde->profondeur > 0 &&
            (objet->position.z < 0 || objet->position.z > monde->profondeur)) {
            objet->vitesse.z = -objet->vitesse.z;
        }
    }
}

void liberer_monde(Monde* monde) {
    free(monde->objets);
    monde->objets = NULL;
    monde->nb_objets = 0;
}