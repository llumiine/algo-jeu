#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "monde.h"
#include "force_brute.h"

int main(void) {
    srand((unsigned) time(NULL));

    Monde monde = creer_monde_uniforme(20, 100.0f, 5.0f, 0);

    printf("=== Monde : %d objets ===\n", monde.nb_objets);
    for (int i = 0; i < monde.nb_objets; i++) {
        Objet* o = monde.objets + i;
        printf("Objet %2d : (%5.1f, %5.1f)\n", o->id, o->position.x, o->position.y);
    }

    int nb = 0;
    Paire* paires = fb_collisions(&monde, &nb);
    printf("\n=== Collisions : %d ===\n", nb);
    for (int i = 0; i < nb; i++) {
        printf("%d touche %d\n", (paires + i)->a, (paires + i)->b);
    }
    free(paires);

    Point min = {0, 0, 0};
    Point max = {50, 50, 0};
    int* dans_boite = fb_objets_dans_boite(&monde, min, max, &nb);
    printf("\n=== Objets dans la boite (0,0)-(50,50) : %d ===\n", nb);
    for (int i = 0; i < nb; i++) {
        printf("%d ", *(dans_boite + i));
    }
    printf("\n");
    free(dans_boite);

    Point centre = {50, 50, 0};
    int* dans_cercle = fb_objets_dans_cercle(&monde, centre, 20.0f, &nb);
    printf("\n=== Objets dans le cercle centre (50,50) rayon 20 : %d ===\n", nb);
    for (int i = 0; i < nb; i++) {
        printf("%d ", *(dans_cercle + i));
    }
    printf("\n");
    free(dans_cercle);

    int k = 3;
    int* proches = fb_k_plus_proches(&monde, centre, k);
    printf("\n=== Les %d objets les plus proches de (50,50) ===\n", k);
    for (int i = 0; i < k; i++) {
        printf("%d ", *(proches + i));
    }
    printf("\n");
    free(proches);

    liberer_monde(&monde);
    return 0;
}