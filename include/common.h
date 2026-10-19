#ifndef COMMON_H
#define COMMON_H

/* Coordonnees d'un point dans l'espace */
typedef struct {
    float x; /* pos horizontale */
    float y; /* vertival */
    float z; /* Pprofondeur */
} Point;

/* Objet circulaire dans le monde */
typedef struct {
    int   id;       /* id objet */
    Point position; /* position objet */
    Point vitesse;  /* deplacement objet axe */
    float rayon;    /* taille objet */
} Objet;

/* Monde contenant tous les objets et ses dimensions*/
typedef struct {
    Objet* objets;    
    int    nb_objets; 
    float  largeur;   
    float  hauteur;   
    float  profondeur; 
} Monde;

typedef struct {
    int a; /* Id premier objet. */
    int b; /* Id du second objet. */
} Paire;

float distance_carre(Point p1, Point p2);

#endif