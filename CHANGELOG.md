# Journal des modifications

Les évolutions notables de Flipper Diver, générées par [git-cliff](https://git-cliff.org) à partir des messages de commit ([Conventional Commits](https://www.conventionalcommits.org/fr/)).

## [0.1](https://github.com/Safenein/flipper-diver/releases/tag/v0.1) — 2026-10-10

### ✨ Nouveautés

- Ajoute le squelette de l'app Flipper Diver ([`c0fa14e`](https://github.com/Safenein/flipper-diver/commit/c0fa14ead60d787045fb1deb6c2723e2744183c8))
- Ajoute la table des 20 stratagèmes ([`99605e0`](https://github.com/Safenein/flipper-diver/commit/99605e0213c162ba3a0d4019da02ab9d6d2993e0))
- **game** : Ajoute le squelette du module de règles ([`e931ff6`](https://github.com/Safenein/flipper-diver/commit/e931ff61abc6e9a4a7ae689f2b479912cac7e51e))
- **game** : Gère la saisie des flèches et le chrono ([`bab2a16`](https://github.com/Safenein/flipper-diver/commit/bab2a1661aa9453bf756fe6083939df50bf01180))
- **game** : Ajoute la fin de manche et ses bonus ([`3711f8e`](https://github.com/Safenein/flipper-diver/commit/3711f8e8ee99c681f3658c342b4d2d66d273b4b6))
- **game** : Ajoute la pause ([`5cd281a`](https://github.com/Safenein/flipper-diver/commit/5cd281a67beed5d047e69eb5d51c193459d28b16))
- **game** : Gère le game over et le record en mémoire ([`de1cf88`](https://github.com/Safenein/flipper-diver/commit/de1cf88ad05c607449d34e8f2bd6e086cfa4418f))
- Branche la boucle de l'app sur le module de règles ([`45b5af4`](https://github.com/Safenein/flipper-diver/commit/45b5af4960e929cf4f0a09d6cde2f00cbf45cb73))
- Ajoute les retours vibreur, LED et son ([`17411ba`](https://github.com/Safenein/flipper-diver/commit/17411ba1055f80aba70e9c1fd793da226651f36a))
- Enregistre le record sur la carte SD ([`0442ee8`](https://github.com/Safenein/flipper-diver/commit/0442ee8764d4b8271a41bc632f03a43c967b35ef))
- Ajoute les icônes des 20 stratagèmes ([`4ef8619`](https://github.com/Safenein/flipper-diver/commit/4ef861990ee63dfa3a8721c9bb231993b0561167))
- **view** : Dessine l'écran de manche définitif ([`33b8037`](https://github.com/Safenein/flipper-diver/commit/33b8037128994aee18f46abf243e231bddc3ac3c))
- **view** : Dessine les écrans de titre, d'annonce, de pause et de fin ([`e6c7830`](https://github.com/Safenein/flipper-diver/commit/e6c7830f70b1d2569522899c4834187730b154d8))
- Met en pause quand l'app perd l'écran ([`6937e25`](https://github.com/Safenein/flipper-diver/commit/6937e25012da27d1103f1a487b3165d92817bbd4))
- Ajoute une instrumentation de mesure désactivable ([`04768fc`](https://github.com/Safenein/flipper-diver/commit/04768fcf6ce5e7c8f3e8fbf0b341cde7a454578d))
- Relève le tas libre au début de chaque partie ([`384df3e`](https://github.com/Safenein/flipper-diver/commit/384df3ef7f699639d84a63f7361ef795ad595233))
- **view** : Habille le titre et le game over aux couleurs du Flipper ([`c4aa19d`](https://github.com/Safenein/flipper-diver/commit/c4aa19d4ee66cf4622137bf9c002ab795249f55a))
- **view** : Remplace la bannière par le crâne du logo de Helldivers ([`b7f4a5b`](https://github.com/Safenein/flipper-diver/commit/b7f4a5b93d327b5d5a0f2d121f635ecb1e82a81e))
- **view** : Reprend l'emblème du logo de Helldivers ([`b6fce0f`](https://github.com/Safenein/flipper-diver/commit/b6fce0f015c3582061d29f07ee6f15ce01b5367d))
- Donne à l'app une icône tirée de l'emblème ([`ae82dee`](https://github.com/Safenein/flipper-diver/commit/ae82dee480ea18a9489ddb7672b8338ac675427c))
- **tools** : Capture les écrans du Flipper par RPC USB ([`baedc65`](https://github.com/Safenein/flipper-diver/commit/baedc6549724998530addb79ff3e7e9e6b3977c3))

### 🐛 Corrections

- **build** : Limite les sources du FAP aux fichiers .c ([`f8051ab`](https://github.com/Safenein/flipper-diver/commit/f8051abc738ab0eff89c7c1e0186b9ca0d937962))
- **view** : Affiche « OK pour jouer » et masque la manche sans record ([`de4c255`](https://github.com/Safenein/flipper-diver/commit/de4c255eda2d6431038935521b76a614d9dcf4b0))
- Passe la pile de l'app à 4 Kio ([`afd6385`](https://github.com/Safenein/flipper-diver/commit/afd6385a9cfd9602d3c5ce805d1c398363d8d68e))

### 📝 Documentation

- Ajoute la spec système, les ADR et le README ([`bf37822`](https://github.com/Safenein/flipper-diver/commit/bf37822b8d4cea5d6dc6f79966011f1a7e042591))
- Indique d'utiliser jj dans CLAUDE.md ([`b41a203`](https://github.com/Safenein/flipper-diver/commit/b41a203573db5e3d85a273b3541a8f0e0e6c0142))
- Remplace prek par jj fix dans CLAUDE.md ([`f9f8a61`](https://github.com/Safenein/flipper-diver/commit/f9f8a61dd0ae9b82095f5d5913998dcaa260a795))
- **specs** : Ajoute le journal des décisions sur les règles du jeu ([`6cd9ab6`](https://github.com/Safenein/flipper-diver/commit/6cd9ab6df3a4274c59009a2ad2ae136ff5c6a990))
- **specs** : Ajoute les exigences des règles du jeu ([`caf1474`](https://github.com/Safenein/flipper-diver/commit/caf14748da8814d6700511c393baee10dc476c84))
- **specs** : Ajoute le design des règles du jeu et l'ADR-0004 ([`81adf5c`](https://github.com/Safenein/flipper-diver/commit/81adf5cda921dc7e07e7c1b359435ea1dac19190))
- **specs** : Ajoute la liste des tâches des règles du jeu ([`35ab99f`](https://github.com/Safenein/flipper-diver/commit/35ab99fd8566d89bbdd2258a01e0447cd03947e7))
- Synchronise la spec système et CLAUDE.md avec le code ([`89ba0f2`](https://github.com/Safenein/flipper-diver/commit/89ba0f24d9d744ba765e4f55c2117ca9afefa75d))
- **specs** : Aligne le design des règles du jeu sur le code ([`5523f2e`](https://github.com/Safenein/flipper-diver/commit/5523f2e1dd829e21a6b8574e09976bcffbaa0f28))
- **specs** : Admet 250 ms d'écart du chrono sur perte de l'écran ([`007bacf`](https://github.com/Safenein/flipper-diver/commit/007bacf5cf3fc7d470505e895bf5e5c16726678f))
- Décrit l'installation, la CI et les releases ([`86f2b9d`](https://github.com/Safenein/flipper-diver/commit/86f2b9d4953a7b9c32894dc2dbe6211944e4dd73))
- Montre les écrans du jeu dans le README ([`7a06307`](https://github.com/Safenein/flipper-diver/commit/7a06307e3c6a3811ea6f8d8ee889d1aa7088b191))

### 🧪 Tests

- Ajoute un harnais de tests unitaires sur PC ([`2cf189e`](https://github.com/Safenein/flipper-diver/commit/2cf189e8b825f180e4b1a855444e3c4bfbc9ebfb))

### 📦 Build

- **devenv** : Ajoute un shell devenv avec ufbt et clang-format ([`b4dec98`](https://github.com/Safenein/flipper-diver/commit/b4dec98b48759df3146a0d5d1761e75370e0a865))
- Partage les tests sur PC avec la CI et ajoute git-cliff ([`bf9944a`](https://github.com/Safenein/flipper-diver/commit/bf9944a4b133d3361e3bf7c45d086605d4e4c592))

### 👷 CI

- Ajoute la CI, le nightly et les releases taggées ([`63729ec`](https://github.com/Safenein/flipper-diver/commit/63729ecabb64228c47e7a094221b8eadfddcfb24))
- Borne la durée des jobs du nightly et des releases ([`10c8a72`](https://github.com/Safenein/flipper-diver/commit/10c8a720d839ae893a53a5a9f0fa5f12d10aae21))

### 🔧 Divers

- Configure clangd pour les tests sur PC ([`599beed`](https://github.com/Safenein/flipper-diver/commit/599beed76d00068a36ebfe7c69a20aec1a32f285))
- Publie le code sous licence GPL-3.0 ([`ef57d35`](https://github.com/Safenein/flipper-diver/commit/ef57d356d57a5b069885621c98ee7a426a4669a1))


