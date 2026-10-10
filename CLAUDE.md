# flipper-diver

FAP Flipper Zero reproduisant le mini-jeu Stratagem Hero de Helldivers 2.
Cible : firmware officiel, canal release (ufbt), en C avec l'API Furi publique.

## Arborescence

- `application.fam` — manifeste de l'app (appid `flipper_diver`).
- `game.c/.h`, `stratagems.c/.h` — règles et table des stratagèmes, en C pur sans
  Furi (ADR-0004) : testés sur PC.
- `view.c/.h` — rendu de chaque phase et table des icônes (même ordre que
  `stratagems.c`).
- `best_score.c/.h` — record sur la carte SD (FlipperFormat).
- `flipper_diver.c` — point d'entrée `flipper_diver_app` : file d'événements, timer,
  notifications, perte d'écran, mesures (`FLIPPER_DIVER_METRICS`).
- `images/` — icônes des stratagèmes (16×16), bannière et touches, générées par
  `tools/make_icons.py` (source ASCII : modifier le script, pas les PNG).
  `flipper_diver.png` — icône 10×10 de l'app.
- `tests/` — tests unitaires sur PC (`tests/test.h`), exclus du FAP.
- `docs/spec/` — spec système et ADR ; `specs/NNN-*/` — specs de fonctionnalités. À
  lire avant tout changement de comportement, à mettre à jour (skill tech-specs) quand
  un changement est livré.

## Commandes (shell devenv)

- `build` (= `ufbt`) — compile vers `dist/flipper_diver.fap`.
- `launch` (= `ufbt launch`) — installe et lance sur le Flipper branché en USB.
- `unit-tests` — compile et lance `tests/test_*.c` sur PC (gnu17, avertissements
  d'ufbt, ASan, UBSan). Un module pur ajouté doit l'être à `pureSources` dans
  `devenv.nix`.
- `fmt` — clang-format.
- `devenv test` — tests sur PC, build ufbt et vérification du formatage.
- `python tools/make_icons.py images` — régénère les icônes.

## Gestion de versions

Dépôt jj colocalisé avec git : utiliser `jj` (jamais `git add`/`git commit`).
Les hooks pre-commit de devenv ne tournent pas sur `jj commit` : lancer
`jj fix` avant de committer. Il applique clang-format aux `*.c`/`*.h`
(configuré dans la config jj du dépôt, hors du dépôt : `jj config list --repo`).

## Pièges

- `ufbt` est un wrapper `buildFHSEnv` (pas de nix-ld sur la machine) autour d'un venv
  hors PATH, `.devenv/state/ufbt-venv` ; voir `docs/spec/adr/0003-*`. Ne pas activer
  `languages.python.venv` : son `ufbt` masquerait le wrapper.
- `ufbt vscode_dist` écrase `.gitignore`, `.clang-format` et `.editorconfig` avec ceux
  de son modèle : restaurer avec `jj restore .gitignore .clang-format .editorconfig`.
- Le motif de sources par défaut d'ufbt (`*.c*`) attrape les `*.cpython-*.pyc` : le
  manifeste se limite à `*.c`, ne pas le rétablir.
- N'utiliser que les en-têtes du SDK release (`~/.ufbt/current/sdk_headers`) : une
  API absente du SDK fait échouer l'APPCHK.
- Le callback d'entrée tourne dans le thread input et le rendu dans le thread GUI :
  l'état du jeu se modifie seulement dans la boucle principale, sous mutex.
- Les polices intégrées n'ont pas d'accents : textes de l'écran en ASCII.
- Tables à une entrée par ligne (`stratagems.c`, icônes de `view.c`) : protégées par
  `/* clang-format off */`, la directive doit être seule dans son commentaire.
