---
status: accepted
date: 2026-10-10
---

# 0001 — Firmware officiel, ufbt et C

## Contexte

L'app doit tourner sur le plus de Flipper Zero possible et pouvoir être publiée sur
le Flipper App Catalog. Les FAP sont liées à une version d'API du firmware.

## Décision

On cible le firmware officiel, canal release, et on construit avec ufbt (SDK
téléchargé, sans cloner le firmware). Le code est en C, avec l'API Furi publique.

## Alternatives

- Firmwares custom (Momentum, Unleashed) — API plus large, mais publics plus
  restreints et app non publiable telle quelle sur le catalogue officiel.
- fbt dans un clone du firmware — plus lourd, utile seulement pour modifier le
  firmware lui-même.
- Rust (flipperzero-rs) — sécurité mémoire, mais bindings non officiels qui suivent
  l'API avec retard.

## Conséquences

- Recompiler à chaque nouvelle version d'API release (`ufbt update` puis `ufbt`).
- Pas d'accès aux API privées ou propres aux firmwares custom.
