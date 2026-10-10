---
status: approved
updated: 2026-10-10
---

# 001 — Règles du jeu : exigences

## Résumé

Transformer le squelette actuel, qui affiche la dernière flèche pressée, en jeu
Stratagem Hero complet : titre, manches chronométrées, barème, progression,
retours vibreur et LED, pause, game over et record sur la carte SD. Les règles
viennent de l'entretien consigné dans [decisions.md](decisions.md) (D1 à D16).

Forme des exigences (EARS) : « Le jeu doit … » (toujours), « Quand … » (événement),
« Tant que … » (état), « Si …, alors … » (cas indésirable). Les identifiants ne
sont jamais renumérotés : une exigence retirée est barrée avec la raison.

Termes : voir le glossaire de `docs/spec/README.md` (stratagème, séquence,
saisie, manche, chrono, erreur).

## Exigences

### Écran titre et navigation

#### REQ-001-1 — Écran titre

Quand l'app démarre, le jeu doit afficher l'écran titre avec le nom du jeu, le
record (score et manche) et l'invite « OK pour jouer ». (D11, D14)

Acceptation :
- Étant donné un record enregistré de 1 250 points en manche 4, quand l'app
  démarre, alors l'écran titre affiche 1 250 et la manche 4.
- Étant donné aucun fichier de record, quand l'app démarre, alors l'écran titre
  affiche un record de 0.

#### REQ-001-2 — Démarrer et quitter depuis le titre

Quand le joueur appuie sur OK à l'écran titre, le jeu doit démarrer une partie à
la manche 1 avec un score de 0. Quand le joueur appuie sur Back à l'écran titre,
l'app doit se fermer. (D14)

Acceptation :
- Depuis le titre, OK → écran « Manche 1 », score 0.
- Depuis le titre, Back → retour au menu du Flipper, sans fuite mémoire signalée
  dans les logs.

### Déroulé d'une manche

#### REQ-001-3 — Annonce de manche

Quand une manche n commence, le jeu doit afficher « Manche n » pendant 1,5 s, le
chrono arrêté, en ignorant les flèches pressées, puis afficher le premier
stratagème. (D14)

Acceptation :
- Une flèche pressée 1 s après l'annonce ne fait pas avancer la séquence et ne
  compte pas comme une erreur.
- Le chrono vaut 10,0 s au moment où le premier stratagème s'affiche.

#### REQ-001-4 — Composition d'une manche

Quand une manche n commence, le jeu doit tirer min(6 + n, 16) stratagèmes au
hasard dans la table, sans jamais placer deux fois de suite le même. (D6)

Acceptation :
- Manche 1 → 7 stratagèmes ; manche 5 → 11 ; manche 10 → 16 ; manche 25 → 16.
- Sur 10 000 manches générées par les tests, aucun stratagème n'apparaît deux
  fois de suite, y compris entre la fin d'une manche et le début de la suivante.

#### REQ-001-5 — Saisie d'une flèche

Tant qu'un stratagème est actif, quand le joueur appuie sur une flèche, le jeu
doit la prendre en compte dès l'appui (`InputTypePress`). Les événements
`InputTypeLong`, `InputTypeRepeat`, `InputTypeShort` et `InputTypeRelease` des
flèches doivent être ignorés. (D4)

Acceptation :
- Maintenir ↑ pendant 2 s sur une séquence ↑↑↓ ne fait avancer la séquence que
  d'une flèche.

#### REQ-001-6 — Flèche correcte

Quand la flèche saisie correspond à la flèche attendue, le jeu doit marquer
cette flèche comme saisie à l'écran et attendre la suivante. (D3)

Acceptation :
- Séquence ↑→↓, saisie ↑ → la première flèche s'affiche pleine, les deux autres
  restent creuses.

#### REQ-001-7 — Erreur de saisie

Si la flèche saisie ne correspond pas à la flèche attendue, alors le jeu doit
remettre la séquence en cours au début, déclencher une vibration courte et la LED
rouge, faire clignoter l'écran, et noter la manche comme non parfaite. Le chrono
ne doit pas être modifié. (D3, D10)

Acceptation :
- Séquence ↑→↓, saisies ↑ puis ↓ → les trois flèches redeviennent creuses, le
  chrono garde sa valeur, et le bonus parfait n'est pas accordé en fin de manche.
- Une erreur sur la première flèche d'une séquence compte aussi comme erreur.

#### REQ-001-8 — Stratagème réussi

Quand la dernière flèche d'une séquence est saisie correctement, le jeu doit
ajouter 5 points par flèche de la séquence au score, ajouter 1,5 s au chrono sans
dépasser 10 s, allumer brièvement la LED verte et passer au stratagème suivant.
(D2, D5, D10)

Acceptation :
- Séquence de 6 flèches réussie avec 4,2 s au chrono → score +30, chrono 5,7 s.
- Séquence réussie avec 9,1 s au chrono → chrono 10,0 s.

#### REQ-001-9 — Chrono

Tant qu'un stratagème est actif et que le jeu n'est pas en pause, le chrono doit
décroître en temps réel. Il ne doit pas décroître pendant l'annonce de manche,
l'écran de fin de manche, la pause ni le game over. (D2, D12, D13, D14)

Acceptation :
- 2 s de pause en pleine manche ne changent pas la valeur du chrono.

#### REQ-001-10 — Game over

Quand le chrono atteint zéro, le jeu doit terminer la partie et afficher l'écran
de game over. Une saisie traitée après que le chrono a atteint zéro ne doit pas
compter. (D2, D14)

Acceptation :
- Chrono à 0,0 s avec une flèche restante → game over, sans les points du
  stratagème en cours.

### Fin de manche

#### REQ-001-11 — Bonus de fin de manche

Quand le dernier stratagème d'une manche n est réussi, le jeu doit ajouter au
score :
- le bonus de manche, 75 + 25 × (n − 1) ;
- le bonus de temps, (temps restant en ms) / 100 arrondi vers le bas, 100 au
  maximum, calculé après la recharge de 1,5 s du dernier stratagème ;
- le bonus parfait, 100, seulement si aucune erreur n'a eu lieu dans la manche.
(D5)

Acceptation :
- Manche 3 finie sans erreur, 4 567 ms restantes → +125 (manche), +45 (temps),
  +100 (parfait).
- Manche 1 finie avec une erreur, 10 000 ms restantes → +75, +100, pas de bonus
  parfait.

#### REQ-001-12 — Écran de fin de manche

Quand une manche est finie, le jeu doit afficher l'un après l'autre le bonus de
manche, le bonus de temps, le bonus parfait (seulement s'il est obtenu) et le
total, puis lancer l'annonce de la manche suivante au bout de 3 s, ou tout de
suite si le joueur appuie sur OK. (D13)

Acceptation :
- Sans appui, l'annonce « Manche n+1 » arrive 3 s après le début de l'écran de
  fin de manche (± 100 ms).
- OK pendant l'écran de fin de manche → annonce immédiate.

### Affichage de la manche

#### REQ-001-13 — Disposition de l'écran de manche

Tant qu'un stratagème est actif, le jeu doit afficher sur 128×64 :
- en haut, une rangée d'icônes 16×16 : l'actif encadré au centre, jusqu'à 2
  stratagèmes réussis de la manche à gauche (barrés), jusqu'à 2 suivants à
  droite ; les emplacements sans stratagème restent vides ;
- au milieu, le nom de l'actif centré, puis sa séquence en flèches dessinées,
  pleines pour les flèches saisies et creuses pour les suivantes ;
- en bas, la jauge du chrono, la manche (« M3 ») et le score. (D8, D9)

Acceptation :
- Premier stratagème de la manche : aucun icône à gauche, 2 icônes à droite.
- Avant-dernier : 2 icônes barrées à gauche, 1 icône à droite.
- Une séquence de 8 flèches tient entièrement dans la largeur de l'écran.
- Le nom le plus long de la table tient sur une ligne sans être coupé.

#### REQ-001-14 — Icônes

Le jeu doit avoir une icône 1 bit de 16×16 pour chaque stratagème de la table,
dessinée pour le projet (pas copiée du jeu original). (D8)

Acceptation :
- Le build échoue, ou un test échoue, si un stratagème de la table n'a pas
  d'icône.

#### REQ-001-15 — Table des stratagèmes

Le jeu doit contenir une table d'au moins 20 stratagèmes, chacun avec un nom, une
icône et une séquence de 3 à 8 flèches, validée par l'utilisateur avant la
livraison. (D7)

Acceptation :
- Un test vérifie que chaque séquence fait de 3 à 8 flèches et que les noms sont
  uniques.

### Pause

#### REQ-001-16 — Mettre en pause

Quand le joueur appuie sur Back pendant l'annonce de manche, une manche ou l'écran
de fin de manche, le jeu doit passer en pause : chrono arrêté, stratagème, icônes
et séquence masqués, avec les invites « OK : reprendre » et « Back : quitter ».
(D12, D13)

Acceptation :
- Pendant la pause, l'écran n'affiche ni nom, ni icône, ni flèche.

#### REQ-001-17 — Reprendre ou quitter depuis la pause

Quand le joueur appuie sur OK en pause, le jeu doit reprendre exactement là où il
s'était arrêté (même chrono, même position dans la séquence, même écran). Quand
le joueur appuie sur Back en pause, l'app doit se fermer sans enregistrer de
record. (D12)

Acceptation :
- Pause à 2/5 flèches et 6,3 s → OK → 2/5 flèches et 6,3 s.
- Partie à 5 000 points, record à 1 000, Back en pause → le record reste à
  1 000.

#### REQ-001-18 — Pause sur perte de l'écran

Si l'app perd l'écran pendant une manche (verrouillage, notification système),
alors le jeu doit se mettre en pause. (D12)

> Sous réserve : la faisabilité avec l'API publique reste à vérifier au design.
> Si elle n'est pas faisable, cette exigence sera barrée avec l'accord de
> l'utilisateur.

Acceptation :
- Verrouiller le Flipper en pleine manche puis le déverrouiller → écran de pause,
  chrono inchangé à 250 ms près (délai de détection de la perte de l'écran ;
  critère assoupli avec l'accord de l'utilisateur le 2026-10-10).

### Game over et record

#### REQ-001-19 — Écran de game over

Quand la partie se termine, le jeu doit afficher le score, la manche atteinte, le
record, et « Nouveau record ! » si le score dépasse strictement l'ancien record.
OK doit ramener à l'écran titre ; Back doit fermer l'app. Une séquence de
notification de fin de partie doit être jouée. (D10, D14)

Acceptation :
- Score égal au record → pas de « Nouveau record ! », record inchangé.

#### REQ-001-20 — Enregistrement du record

Quand la partie se termine avec un score strictement supérieur au record, le jeu
doit enregistrer le score et la manche dans `APP_DATA_PATH("best.fff")` au format
FlipperFormat (`Filetype`, `Version: 1`, `Score`, `Round`). Il ne doit pas écrire
sur la carte SD dans les autres cas. (D11)

Acceptation :
- Après un nouveau record, relancer l'app affiche ce record à l'écran titre.
- Une partie qui ne bat pas le record ne modifie pas la date du fichier.

#### REQ-001-21 — Record illisible

Si le fichier de record est absent, illisible, ou d'une version autre que 1, alors
le jeu doit considérer le record comme nul, sans message d'erreur bloquant, et
écraser le fichier au prochain record. Si l'écriture du record échoue (carte SD
absente ou pleine), alors le jeu doit continuer normalement et le consigner dans
les logs. (D11)

Acceptation :
- Fichier contenant `Version: 2` → record affiché à 0.
- Sans carte SD, une partie se joue et se termine normalement.

### Retours physiques

#### REQ-001-22 — Respect des réglages du Flipper

Le jeu doit produire tous ses sons, vibrations et LED par le service de
notification, de façon que le mode silencieux et le volume du Flipper
s'appliquent. (D10)

Acceptation :
- Avec la vibration désactivée dans les réglages du Flipper, une erreur ne fait
  pas vibrer.

### Qualité

#### REQ-001-23 — Logique de jeu testable sur PC

Le jeu doit isoler ses règles (séquences, chrono, barème, progression, tirage)
dans un module C sans dépendance au firmware, avec le temps et l'aléa fournis de
l'extérieur. Ses tests unitaires doivent tourner sur PC sous ASan et UBSan via
`devenv test`. Ils ne doivent pas être inclus dans le FAP. (D15)

Acceptation :
- `devenv test` compile et lance les tests sur PC, et échoue si l'un d'eux
  échoue.
- Le FAP construit par `ufbt` ne contient aucun symbole de test.

#### REQ-001-24 — Réactivité

Tant qu'un stratagème est actif, le jeu doit rafraîchir l'écran au moins 25 fois
par seconde, et afficher une flèche saisie au plus 40 ms après l'appui. (D16)

Acceptation :
- Mesure sur l'appareil à l'aide des horodatages dans les logs (`furi_get_tick`) :
  intervalle entre deux rendus ≤ 40 ms, et entre l'appui et le rendu ≤ 40 ms.

#### REQ-001-25 — Pas d'allocation en jeu

Le jeu ne doit faire aucune allocation dynamique après l'initialisation de l'app,
ni dans le callback de rendu. (invariant de la spec système, D16)

Acceptation :
- La mémoire libre du tas, relevée au début et à la fin d'une partie de 5 manches,
  est identique (log `memmgr_get_free_heap`).

## Non-objectifs

- Réglages dans l'app (durée du chrono, son, difficulté) — D2, D10.
- Tableau des meilleurs scores ou saisie d'un nom — D11.
- Liste complète des stratagèmes du jeu — D7.
- Accélération du chrono au fil des manches — D6.
- Compatibilité testée avec les firmwares custom — spec système.

## Questions ouvertes

- [ ] Valider les séquences de la table des stratagèmes (REQ-001-15). (safenein)
- [ ] Faisabilité de la pause sur perte de l'écran (REQ-001-18). (Claude, au design)
- [ ] Moyen d'exclure les tests du FAP (REQ-001-23). (Claude, au design)
- [x] Bonus de temps calculé après la recharge du dernier stratagème
  (REQ-001-11) : choix de Claude, validé avec les exigences. (safenein)
- [ ] Valeurs du chrono et du barème à ajuster après des parties sur l'appareil.
  (safenein)
