# algo-jeu


### Installer Git

```powershell
winget install --id Git.Git -e --source winget
```

Fermez complètement VS Code, rouvrez-le, puis vérifiez :

```powershell
git --version
```

Configurez votre identité (avec l'email de votre compte GitHub) :

```powershell
git config --global user.name "Votre Prénom"
git config --global user.email "votre-email-github@exemple.com"
```

### Installer le compilateur C (gcc)

```powershell
winget install BrechtSanders.WinLibs.POSIX.UCRT
```

Fermez complètement VS Code, rouvrez-le, puis vérifiez :

```powershell
gcc --version
```

**Si `gcc` n'est pas reconnu**, copiez ces lignes ensemble dans PowerShell.
Elles retrouvent gcc et l'ajoutent au PATH de Windows de façon permanente :

```powershell
$gcc = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -Filter gcc.exe -ErrorAction SilentlyContinue | Select-Object -First 1
$bin = $gcc.DirectoryName
$cheminUtilisateur = [Environment]::GetEnvironmentVariable("Path", "User")
[Environment]::SetEnvironmentVariable("Path", "$cheminUtilisateur;$bin", "User")
$env:Path += ";$bin"
gcc --version
```

---

## Compiler et lancer

Programme de démonstration :

```powershell
gcc -O2 -Iinclude src/*.c -o algo-jeu.exe
./algo-jeu.exe
```

## Benchmarks

Le programme `bench/benchmark.c` compare tous les algorithmes déclarés dans
`src/algorithmes.c` (force brute, grille, ...) en faisant varier le nombre d'objets
(500 à 50 000), la répartition (uniforme / amas), la taille des objets
(homogène / variable) et la dimension (2D / 3D). Pour chaque cas il mesure le temps
de construction, la mémoire, le temps de détection des collisions, le temps des
requêtes (boîte, cercle, 10 plus proches voisins) et, en scène dynamique, le coût de
mise à jour par frame.

```powershell
gcc -O2 -Iinclude bench/benchmark.c src/monde.c src/force_brute.c src/grille.c src/algorithmes.c src/chrono.c -o benchmark.exe
./benchmark.exe              # complet (~2 min), écrit results/benchmark.csv
./benchmark.exe --rapide     # quelques tailles seulement
./benchmark.exe --fb-max 50000   # la force brute est sautée au-delà de 20 000 objets par défaut
```

Graphiques (un PNG par mesure dans `results/`, en log-log avec la pente de chaque courbe) :

```powershell
pip install matplotlib
python scripts/graphiques.py results/benchmark.csv results
```

Pour ajouter un algorithme aux benchmarks : écrire ses fonctions d'adaptation dans
`src/algorithmes.c` et l'ajouter au tableau `ALGORITHMES` (voir le TODO), puis ajouter
son `.c` à la ligne de compilation.
