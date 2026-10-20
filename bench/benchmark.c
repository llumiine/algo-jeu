/*
 * Benchmarks : compare tous les algorithmes de ALGORITHMES (algorithmes.c)
 * en faisant varier le nombre d'objets, leur repartition (uniforme / amas),
 * leur taille (homogene / variable) et la dimension (2D / 3D).
 *
 * Pour chaque cas on mesure (mediane de plusieurs mesures) :
 *   - le temps de construction de la structure
 *   - la memoire occupee par la structure
 *   - le temps de detection de toutes les collisions
 *   - le temps median d'une requete boite, cercle et k plus proches voisins
 *   - en scene dynamique : le temps de mise a jour par frame, et le temps
 *     d'une frame complete (mise a jour + collisions)
 *
 * Les resultats sont ecrits dans un fichier CSV (results/benchmark.csv par defaut),
 * a tracer avec scripts/graphiques.py.
 *
 * Usage : benchmark [--rapide] [--sortie fichier.csv] [--fb-max N]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "monde.h"
#include "algorithmes.h"
#include "chrono.h"

#define ESPACEMENT       5.0f   /* distance moyenne entre objets : la densite reste constante quand n augmente */
#define RAYON_HOMOGENE   1.0f
#define RAYON_MIN        0.5f   /* tailles variables : de RAYON_MIN a RAYON_MIN * RAPPORT_RAYONS */
#define RAPPORT_RAYONS   20.0f
#define NB_AMAS          10
#define TAILLE_REQUETE   10.0f  /* demi-cote de la boite et rayon du cercle des requetes */
#define K                10
#define NB_REQUETES      200
#define NB_FRAMES        10
#define DT               1.0f

typedef struct {
    int en_3d;
    int amas;
    int tailles_variables;
} Scenario;

typedef struct {
    double construction_ms;
    size_t memoire;
    double collisions_ms;
    int    nb_collisions;
    double boite_us;
    double cercle_us;
    double kppv_us;
    double mise_a_jour_ms;
    double frame_ms;
} Mesures;

static int comparer_doubles(const void* a, const void* b) {
    double d1 = *(const double*) a;
    double d2 = *(const double*) b;
    if (d1 < d2) return -1;
    if (d1 > d2) return 1;
    return 0;
}

/* Mediane de n temps (trie le tableau). Contrairement a la moyenne, elle n'est pas
   faussee par une mesure ou le programme a ete interrompu par le systeme. */
static double mediane(double* temps, int n) {
    if (n <= 0) return 0;
    qsort(temps, n, sizeof(double), comparer_doubles);
    if (n % 2 == 1) return *(temps + n / 2);
    return (*(temps + n / 2 - 1) + *(temps + n / 2)) / 2;
}

static float hasard(float min, float max) {
    return min + (max - min) * ((float) rand() / (float) RAND_MAX);
}

/* Beaucoup de petits objets et quelques gros : r = RAYON_MIN * RAPPORT^(u^2) */
static void rendre_tailles_variables(Monde* monde) {
    for (int i = 0; i < monde->nb_objets; i++) {
        float u = hasard(0, 1);
        (monde->objets + i)->rayon = RAYON_MIN * powf(RAPPORT_RAYONS, u * u);
    }
}

static Monde creer_scenario(Scenario sc, int n) {
    float taille = sc.en_3d ? ESPACEMENT * cbrtf((float) n) : ESPACEMENT * sqrtf((float) n);
    Monde monde = sc.amas ? creer_monde_amas(n, taille, RAYON_HOMOGENE, NB_AMAS, sc.en_3d)
                          : creer_monde_uniforme(n, taille, RAYON_HOMOGENE, sc.en_3d);
    if (sc.tailles_variables) rendre_tailles_variables(&monde);
    return monde;
}

static Monde copier_monde(Monde* source) {
    Monde copie = *source;
    copie.objets = malloc(source->nb_objets * sizeof(Objet));
    if (copie.objets == NULL) {
        copie.nb_objets = 0;
        return copie;
    }
    memcpy(copie.objets, source->objets, source->nb_objets * sizeof(Objet));
    return copie;
}

/* Chaque mesure est repetee et on garde la mediane des temps (en secondes) */
static Mesures mesurer(const Algorithme* algo, Monde* reference, Point* centres, int repetitions) {
    Mesures m = {0};
    double t;

    /* tableau assez grand pour toutes les series de temps */
    int taille_temps = NB_REQUETES;
    if (repetitions > taille_temps) taille_temps = repetitions;
    if (NB_FRAMES > taille_temps) taille_temps = NB_FRAMES;
    double* temps = malloc(taille_temps * sizeof(double));
    double* temps_frame = malloc(NB_FRAMES * sizeof(double));
    if (temps == NULL || temps_frame == NULL) {
        free(temps);
        free(temps_frame);
        return m;
    }

    /* on travaille sur une copie : la partie dynamique deplace les objets */
    Monde monde = copier_monde(reference);

    /* construction */
    void* s = NULL;
    for (int r = 0; r < repetitions; r++) {
        if (s != NULL) algo->liberer(s);
        t = chrono_maintenant();
        s = algo->construire(&monde);
        *(temps + r) = chrono_maintenant() - t;
    }
    m.construction_ms = mediane(temps, repetitions) * 1e3;
    if (s == NULL) {
        fprintf(stderr, "  %s : construction impossible\n", algo->nom);
        free(monde.objets);
        free(temps);
        free(temps_frame);
        return m;
    }
    m.memoire = algo->memoire(s);

    /* collisions */
    for (int r = 0; r < repetitions; r++) {
        int nb = 0;
        t = chrono_maintenant();
        Paire* paires = algo->collisions(s, &nb);
        *(temps + r) = chrono_maintenant() - t;
        free(paires);
        m.nb_collisions = nb;
    }
    m.collisions_ms = mediane(temps, repetitions) * 1e3;

    /* requetes, centrees sur des objets tires au hasard (les memes pour tous les algos) */
    for (int q = 0; q < NB_REQUETES; q++) {
        Point c = *(centres + q);
        Point min = { c.x - TAILLE_REQUETE, c.y - TAILLE_REQUETE, c.z - TAILLE_REQUETE };
        Point max = { c.x + TAILLE_REQUETE, c.y + TAILLE_REQUETE, c.z + TAILLE_REQUETE };
        int nb = 0;
        t = chrono_maintenant();
        int* res = algo->objets_dans_boite(s, min, max, &nb);
        *(temps + q) = chrono_maintenant() - t;
        free(res);
    }
    m.boite_us = mediane(temps, NB_REQUETES) * 1e6;

    for (int q = 0; q < NB_REQUETES; q++) {
        int nb = 0;
        t = chrono_maintenant();
        int* res = algo->objets_dans_cercle(s, *(centres + q), TAILLE_REQUETE, &nb);
        *(temps + q) = chrono_maintenant() - t;
        free(res);
    }
    m.cercle_us = mediane(temps, NB_REQUETES) * 1e6;

    for (int q = 0; q < NB_REQUETES; q++) {
        t = chrono_maintenant();
        int* res = algo->k_plus_proches(s, *(centres + q), K);
        *(temps + q) = chrono_maintenant() - t;
        free(res);
    }
    m.kppv_us = mediane(temps, NB_REQUETES) * 1e6;

    /* scene dynamique : a chaque frame on deplace, on met a jour, on detecte */
    for (int f = 0; f < NB_FRAMES; f++) {
        deplacer_objets(&monde, DT);

        int nb = 0;
        t = chrono_maintenant();
        algo->mettre_a_jour(s);
        *(temps + f) = chrono_maintenant() - t;
        Paire* paires = algo->collisions(s, &nb);
        *(temps_frame + f) = chrono_maintenant() - t;
        free(paires);
    }
    m.mise_a_jour_ms = mediane(temps, NB_FRAMES) * 1e3;
    m.frame_ms = mediane(temps_frame, NB_FRAMES) * 1e3;

    algo->liberer(s);
    free(monde.objets);
    free(temps);
    free(temps_frame);
    return m;
}

int main(int argc, char** argv) {
    const char* sortie = "results/benchmark.csv";
    int rapide = 0;
    int fb_max = 20000;   /* au-dela, la force brute (O(n^2)) prend trop de temps */

    for (int i = 1; i < argc; i++) {
        if (strcmp(*(argv + i), "--rapide") == 0) {
            rapide = 1;
        } else if (strcmp(*(argv + i), "--sortie") == 0 && i + 1 < argc) {
            sortie = *(argv + ++i);
        } else if (strcmp(*(argv + i), "--fb-max") == 0 && i + 1 < argc) {
            fb_max = atoi(*(argv + ++i));
        } else {
            fprintf(stderr, "Usage : %s [--rapide] [--sortie fichier.csv] [--fb-max N]\n", *argv);
            return 1;
        }
    }

    int tailles_completes[] = { 500, 1000, 2000, 5000, 10000, 20000, 50000 };
    int tailles_rapides[] = { 500, 2000, 10000 };
    int* tailles = rapide ? tailles_rapides : tailles_completes;
    int nb_tailles = rapide ? 3 : 7;
    int repetitions = rapide ? 1 : 5;

    FILE* csv = fopen(sortie, "w");
    if (csv == NULL) {
        fprintf(stderr, "Impossible d'ouvrir %s (lancer depuis la racine du projet ?)\n", sortie);
        return 1;
    }
    fprintf(csv, "algorithme,dimension,distribution,tailles,n,construction_ms,memoire_octets,"
                 "collisions_ms,nb_collisions,boite_us,cercle_us,kppv_us,mise_a_jour_ms,frame_ms\n");

    Point* centres = malloc(NB_REQUETES * sizeof(Point));
    if (centres == NULL) return 1;

    int graine = 1;
    for (int en_3d = 0; en_3d <= 1; en_3d++) {
        for (int amas = 0; amas <= 1; amas++) {
            for (int variables = 0; variables <= 1; variables++) {
                Scenario sc = { en_3d, amas, variables };
                const char* dim = en_3d ? "3D" : "2D";
                const char* distribution = amas ? "amas" : "uniforme";
                const char* tailles_obj = variables ? "variable" : "homogene";

                for (int t = 0; t < nb_tailles; t++) {
                    int n = *(tailles + t);

                    /* meme graine => meme monde a chaque lancement */
                    srand(graine++);
                    Monde monde = creer_scenario(sc, n);
                    if (monde.nb_objets == 0) continue;
                    for (int q = 0; q < NB_REQUETES; q++) {
                        *(centres + q) = (monde.objets + rand() % monde.nb_objets)->position;
                    }

                    printf("%s %-8s %-8s n=%-6d", dim, distribution, tailles_obj, n);
                    fflush(stdout);

                    int collisions_reference = -1;
                    for (int a = 0; a < NB_ALGORITHMES; a++) {
                        const Algorithme* algo = ALGORITHMES + a;
                        if (strcmp(algo->nom, "force_brute") == 0 && n > fb_max) continue;

                        Mesures m = mesurer(algo, &monde, centres, repetitions);

                        fprintf(csv, "%s,%s,%s,%s,%d,%.6f,%zu,%.6f,%d,%.6f,%.6f,%.6f,%.6f,%.6f\n",
                                algo->nom, dim, distribution, tailles_obj, n,
                                m.construction_ms, m.memoire, m.collisions_ms, m.nb_collisions,
                                m.boite_us, m.cercle_us, m.kppv_us, m.mise_a_jour_ms, m.frame_ms);
                        fflush(csv);

                        printf("  %s: %.2f ms", algo->nom, m.collisions_ms);
                        fflush(stdout);

                        /* verification rapide : tout le monde doit trouver le meme nombre de collisions */
                        if (collisions_reference < 0) {
                            collisions_reference = m.nb_collisions;
                        } else if (m.nb_collisions != collisions_reference) {
                            printf(" [ATTENTION %d collisions au lieu de %d]", m.nb_collisions, collisions_reference);
                        }
                    }
                    printf("\n");

                    liberer_monde(&monde);
                }
            }
        }
    }

    free(centres);
    fclose(csv);
    printf("Resultats ecrits dans %s\n", sortie);
    return 0;
}
