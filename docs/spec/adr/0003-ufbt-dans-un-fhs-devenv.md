---
status: accepted
date: 2026-10-10
---

# 0003 — ufbt dans un environnement FHS sous devenv

## Contexte

ufbt n'est pas packagé dans nixpkgs. Il télécharge dans `~/.ufbt` une chaîne ARM
(gcc, python, openocd) liée dynamiquement à la glibc, que NixOS ne peut pas exécuter
sans nix-ld — absent sur la machine de développement.

## Décision

`devenv.nix` installe ufbt (version figée dans `requirements.txt`) avec pip dans un
venv hors PATH (`.devenv/state/ufbt-venv`), et expose une commande `ufbt` qui le
lance dans un `pkgs.buildFHSEnv`.

## Alternatives

- Activer nix-ld sur le système — touche la configuration NixOS hors du projet.
- `FBT_TOOLCHAIN_PATH` vers `gcc-arm-embedded` de nixpkgs — gcc différent de la
  version officielle et outils annexes (python, openocd) à reconstituer.
- venv standard de devenv sur le PATH — son `ufbt` masque le wrapper FHS.

## Conséquences

- `ufbt` fonctionne dans le shell devenv sans réglage système.
- Hors devenv (CI GitHub, autre distribution), ufbt s'utilise normalement.
- Ajouter une bibliothèque à `targetPkgs` si un outil de la chaîne réclame une `.so`
  manquante.
