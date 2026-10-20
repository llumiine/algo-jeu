"""
Trace les graphiques a partir du CSV produit par le programme benchmark.

Usage : python scripts/graphiques.py [results/benchmark.csv] [dossier_sortie]

Pour chaque mesure, une figure avec un graphique par scenario
(dimension x repartition x taille des objets), en echelle log-log.
La pente de chaque courbe est indiquee dans la legende : en log-log,
une pente de 1 correspond a O(n), une pente de 2 a O(n^2), une pente
de 0 a un temps constant. C'est ce qu'on compare a la complexite theorique.

Ne demande que matplotlib (pip install matplotlib).
"""
import csv
import math
import os
import sys
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

MESURES = [
    ("construction_ms", "Temps de construction", "ms"),
    ("collisions_ms", "Detection de toutes les collisions", "ms"),
    ("boite_us", "Requete boite (par requete)", "µs"),
    ("cercle_us", "Requete cercle (par requete)", "µs"),
    ("kppv_us", "10 plus proches voisins (par requete)", "µs"),
    ("mise_a_jour_ms", "Mise a jour apres deplacement (par frame)", "ms"),
    ("frame_ms", "Frame dynamique : mise a jour + collisions", "ms"),
    ("memoire_octets", "Memoire de la structure", "octets"),
]

# Couleurs fixes par algorithme (la couleur suit l'algorithme, pas son rang)
COULEURS = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4", "#008300", "#6250d6", "#e34948"]
MARQUEURS = ["o", "s", "^", "D", "v", "P", "X", "*"]

ENCRE = "#52514e"
GRILLE_FOND = "#e4e3df"


def lire(chemin):
    with open(chemin, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def pente(xs, ys):
    """Pente de la droite des moindres carres en log-log."""
    points = [(math.log(x), math.log(y)) for x, y in zip(xs, ys) if x > 0 and y > 0]
    if len(points) < 2:
        return None
    mx = sum(p[0] for p in points) / len(points)
    my = sum(p[1] for p in points) / len(points)
    num = sum((p[0] - mx) * (p[1] - my) for p in points)
    den = sum((p[0] - mx) ** 2 for p in points)
    return num / den if den else None


def style_axes(ax):
    for cote in ("top", "right"):
        ax.spines[cote].set_visible(False)
    for cote in ("left", "bottom"):
        ax.spines[cote].set_color(GRILLE_FOND)
    ax.tick_params(colors=ENCRE, labelsize=8)
    ax.grid(True, which="major", color=GRILLE_FOND, linewidth=0.8)
    ax.set_axisbelow(True)


def main():
    chemin = sys.argv[1] if len(sys.argv) > 1 else "results/benchmark.csv"
    dossier = sys.argv[2] if len(sys.argv) > 2 else os.path.dirname(chemin) or "."
    os.makedirs(dossier, exist_ok=True)

    lignes = lire(chemin)
    algorithmes = list(dict.fromkeys(l["algorithme"] for l in lignes))
    couleur = {a: COULEURS[i % len(COULEURS)] for i, a in enumerate(algorithmes)}
    marqueur = {a: MARQUEURS[i % len(MARQUEURS)] for i, a in enumerate(algorithmes)}

    scenarios = list(dict.fromkeys((l["dimension"], l["distribution"], l["tailles"]) for l in lignes))

    # donnees[scenario][algo] = liste de lignes triees par n
    donnees = defaultdict(lambda: defaultdict(list))
    for l in lignes:
        donnees[(l["dimension"], l["distribution"], l["tailles"])][l["algorithme"]].append(l)
    for par_algo in donnees.values():
        for liste in par_algo.values():
            liste.sort(key=lambda l: int(l["n"]))

    colonnes = min(4, len(scenarios))
    rangees = math.ceil(len(scenarios) / colonnes)

    for cle, titre, unite in MESURES:
        fig, axes = plt.subplots(rangees, colonnes, figsize=(4 * colonnes, 3.4 * rangees),
                                 squeeze=False, sharex=True)
        for i, sc in enumerate(scenarios):
            ax = axes[i // colonnes][i % colonnes]
            style_axes(ax)
            for algo in algorithmes:
                liste = donnees[sc].get(algo, [])
                xs = [int(l["n"]) for l in liste]
                ys = [float(l[cle]) for l in liste]
                points = [(x, y) for x, y in zip(xs, ys) if y > 0]
                if not points:
                    continue
                p = pente(xs, ys)
                etiquette = algo if p is None else f"{algo} (pente {p:.2f})"
                ax.plot([x for x, _ in points], [y for _, y in points], color=couleur[algo],
                        marker=marqueur[algo], markersize=5, linewidth=2, label=etiquette)
            ax.set_xscale("log")
            ax.set_yscale("log")
            ax.set_title(" / ".join(sc), fontsize=9, color=ENCRE)
            ax.legend(fontsize=7, frameon=False, labelcolor=ENCRE)
            if i // colonnes == rangees - 1:
                ax.set_xlabel("nombre d'objets n", fontsize=8, color=ENCRE)
            if i % colonnes == 0:
                ax.set_ylabel(unite, fontsize=8, color=ENCRE)
        for j in range(len(scenarios), rangees * colonnes):
            axes[j // colonnes][j % colonnes].set_visible(False)

        fig.suptitle(titre, fontsize=12, color="#0b0b0b")
        fig.tight_layout()
        sortie = os.path.join(dossier, f"{cle}.png")
        fig.savefig(sortie, dpi=120)
        plt.close(fig)
        print("ecrit", sortie)


if __name__ == "__main__":
    main()
