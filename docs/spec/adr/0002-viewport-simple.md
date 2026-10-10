---
status: accepted
date: 2026-10-10
---

# 0002 — Un seul ViewPort plutôt qu'un ViewDispatcher

## Contexte

Le jeu a quelques écrans (titre, manche, pause, game over) mais un seul rendu plein
écran redessiné en continu pendant une manche, piloté par une machine à états.

## Décision

On utilise un unique `ViewPort` plein écran, une `FuriMessageQueue` pour les
entrées et une machine à états dans la boucle principale ; chaque écran est un cas
du callback de rendu.

## Alternatives

- `ViewDispatcher` + `SceneManager` — adapté aux apps à menus et widgets standard,
  mais beaucoup de code d'enrobage pour des écrans tous dessinés à la main.

## Conséquences

- Code court et contrôle total du rendu et du timing.
- Si des menus standard (réglages, liste de scores) arrivent, revoir cette décision.
