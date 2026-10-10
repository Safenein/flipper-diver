# flipper-diver

FAP Flipper Zero reproduisant le mini-jeu Stratagem Hero de Helldivers 2.
Cible : firmware officiel, canal release (ufbt), en C avec l'API Furi publique.

## Arborescence

- `application.fam` — manifeste de l'app (appid `flipper_diver`).
- `flipper_diver.c` — point d'entrée `flipper_diver_app` : ViewPort, file d'entrées, rendu.
- `flipper_diver.png` — icône 10×10 1 bit.
- `docs/spec/` — spec système et ADR ; à lire avant tout changement de comportement,
  à mettre à jour (skill tech-specs) quand un changement est livré.

## Commandes (shell devenv)

- `build` (= `ufbt`) — compile vers `dist/flipper_diver.fap`.
- `launch` (= `ufbt launch`) — installe et lance sur le Flipper branché en USB.
- `fmt` — clang-format ; un hook pre-commit le vérifie aussi.
- `devenv test` — build + vérification du formatage.

## Gestion de versions

Dépôt jj colocalisé avec git : utiliser `jj` (jamais `git add`/`git commit`).
Les hooks pre-commit de devenv ne tournent pas sur `jj commit` : lancer
`prek run -a` (ou `fmt`) avant de committer.

## Pièges

- `ufbt` est un wrapper `buildFHSEnv` (pas de nix-ld sur la machine) autour d'un venv
  hors PATH, `.devenv/state/ufbt-venv` ; voir `docs/spec/adr/0003-*`. Ne pas activer
  `languages.python.venv` : son `ufbt` masquerait le wrapper.
- `ufbt vscode_dist` écrase `.gitignore`, `.clang-format` et `.editorconfig` avec ceux
  de son modèle : restaurer avec `git checkout -- .gitignore .clang-format .editorconfig`.
- N'utiliser que les en-têtes du SDK release (`~/.ufbt/current/sdk_headers`) : une
  API absente du SDK fait échouer l'APPCHK.
- Le callback d'entrée tourne dans le thread input et le rendu dans le thread GUI :
  l'état du jeu se modifie seulement dans la boucle principale, sous mutex.
