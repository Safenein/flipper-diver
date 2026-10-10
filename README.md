# Flipper Diver

Le mini-jeu des stratagèmes de Helldivers 2 (« Stratagem Hero ») sur Flipper Zero :
recopie la séquence de flèches au pavé directionnel avant la fin du chrono.

Cible : firmware officiel, canal release.

## Build

Avec [devenv](https://devenv.sh) (et direnv, ou `devenv shell`) :

```sh
build    # ufbt : produit dist/flipper_diver.fap
launch   # installe et lance l'app sur un Flipper branché en USB
fmt      # clang-format sur les sources C
```

Sans devenv : `pip install -r requirements.txt` puis `ufbt` / `ufbt launch`.

## Installation manuelle

Copier `dist/flipper_diver.fap` dans `apps/Games/` sur la carte SD du Flipper.

## Documentation

Spec technique : [docs/spec/](docs/spec/README.md).
