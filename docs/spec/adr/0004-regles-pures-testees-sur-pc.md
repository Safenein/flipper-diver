---
status: accepted
date: 2026-10-10
---

# 0004 — Règles du jeu dans un module pur, testé sur PC

## Contexte

Le barème, le chrono et la progression doivent rester justes au fil des
changements (`specs/001-regles-du-jeu`, D15). Tester sur l'appareil demande un
Flipper branché et ne donne pas de non-régression automatique.

## Décision

Les règles vivent dans `game.c` et `stratagems.c`, en C sans aucune dépendance à
Furi ni au SDK. Le temps écoulé, les touches et la graine du générateur
aléatoire sont fournis par l'appelant. Ces fichiers sont compilés deux fois : dans
le FAP par ufbt, et sur PC avec les tests de `tests/`, sous ASan et UBSan, via
`devenv test`. `application.fam` exclut `tests/` des sources du FAP.

## Alternatives

- Tests sur l'appareil seulement — pas de non-régression automatique.
- Framework de tests unitaires du firmware — lourd à mettre en place avec ufbt,
  et demande un Flipper branché.

## Conséquences

- Aucune en-tête Furi ne doit être incluse dans `game.c` ni `stratagems.c` ; la
  compilation sur PC le vérifie.
- Les icônes et tout ce qui touche la GUI restent côté app, dans des tableaux
  parallèles à la table des stratagèmes.
- Deux compilateurs (arm-none-eabi et gcc du PC) doivent accepter le même code :
  s'en tenir au C standard et à `stdint.h` et `stdbool.h`.
