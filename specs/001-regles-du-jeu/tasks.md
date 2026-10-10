---
status: approved
updated: 2026-10-10
---

# 001 — Règles du jeu : tâches

Une tâche = un commit jj (`jj fix` avant), qui laisse `devenv test` vert : tests
sur PC, build ufbt et formatage. Les tests arrivent avec ou avant le code qu'ils
couvrent. « Manuel » = vérifié sur l'appareil avec `launch`.

## Socle

- [x] 1. Harnais de tests sur PC : `tests/test.h` (macros `CHECK`, code de sortie),
  un premier test trivial, un script devenv `unit-tests` (gnu17, mêmes
  avertissements qu'ufbt, ASan et UBSan) appelé par `enterTest`, et
  `sources=["*.c*", "!tests"]` dans `application.fam`. — REQ-001-23 — verify :
  `devenv test` échoue si on casse le test exprès ;
  `nm dist/debug/flipper_diver_d.elf | grep test_` ne trouve rien (le `nm` du
  PC lit l'ELF ARM).
- [x] 2. Table des stratagèmes `stratagems.h/.c` : environ 20 entrées (nom,
  séquence, longueur), rédigées par Claude, avec les tests de la table
  (séquences de 3 à 8 flèches, noms uniques, noms de 21 caractères au plus).
  — REQ-001-15, REQ-001-13 (noms) — verify : `unit-tests`.
- [ ] **Point d'arrêt** : l'utilisateur valide les séquences de la table. Les
  corrections éventuelles font l'objet d'un commit à part.

## Règles (`game.c`, sans Furi)

- [x] 3. Squelette de `game.h/.c` : phases, `game_init`, `game_advance`,
  `game_key`, `GameEvents` ; titre → annonce de 1,5 s pendant laquelle les
  flèches sont ignorées ; composition de manche min(6 + n, 16) avec xorshift32
  et décalage anti-doublon. — REQ-001-2, 3, 4 — verify : tests de taille des
  manches 1, 5, 10 et 25 ; test de propriété sur 10 000 manches à graine fixe.
- [x] 4. Saisie et chrono : flèche correcte, erreur (remise à zéro, manche non
  parfaite, chrono intact), stratagème réussi (+5 par flèche, +1,5 s plafonnées
  à 10 s), décompte, game over à 0, saisie ignorée après le game over. —
  REQ-001-6, 7, 8, 9, 10 — verify : un test par critère d'acceptation de ces
  REQ (valeurs reprises telles quelles).
- [x] 5. Fin de manche : bonus de manche, de temps (après recharge) et parfait ;
  phase `RoundEnd` de 3 s ou OK, puis annonce de la manche suivante. —
  REQ-001-11, 12 (logique) — verify : tests « manche 3, 4 567 ms, sans erreur →
  +125, +45, +100 » et « manche 1 avec erreur, 10 000 ms → +75, +100 ».
- [x] 6. Pause : Back en annonce, manche ou fin de manche ; OK reprend à
  l'identique ; Back quitte (`Quit`) sans `NewRecord` ; `game_pause`. —
  REQ-001-16, 17, 18 (logique) — verify : test « pause à 2/5 flèches et 6,3 s →
  reprise identique » ; test « Back en pause → Quit, record inchangé ».
- [x] 7. Game over et record en mémoire : `NewRecord` seulement si le score
  dépasse strictement le record ; OK → titre, Back → `Quit`. — REQ-001-19
  (logique) — verify : tests « score égal au record → pas de NewRecord » et
  « score supérieur → NewRecord ».

## App Flipper

- [ ] 8. Boucle de l'app branchée sur `game.c` : `AppEvent` (entrée ou tick),
  `FuriTimer` de 33 ms, filtrage Press pour les flèches et Short pour OK et
  Back, `game_advance` avant `game_key`, `view_port_update` immédiat après une
  saisie, sortie sur `Quit`. Rendu texte minimal provisoire (phase, séquence en
  texte, chrono, score). — REQ-001-5, 9, 24 (structure) — verify : manuel, une
  partie complète jouable en texte, et sortie propre sans fuite dans les logs.
  *Code fait et compilé ; vérification sur l'appareil en attente.*
- [ ] 9. Notifications : séquence d'erreur propre (vibreur 100 ms + LED rouge),
  `sequence_blink_green_10`, `sequence_success`, `sequence_error`. —
  REQ-001-7 (retour), REQ-001-22 — verify : manuel, y compris avec la
  vibration coupée dans les réglages du Flipper.
  *Code fait et compilé ; vérification sur l'appareil en attente.*
- [ ] 10. Record sur la carte SD `best_score.h/.c` : chargement au démarrage,
  écriture sur `NewRecord`, tolérance aux fichiers absents, corrompus ou de
  version 2, et à l'absence de carte SD. — REQ-001-1 (chargement), 20, 21 —
  verify : manuel, les quatre cas de REQ-001-20 et 21.
  *Code fait et compilé ; vérification sur l'appareil en attente.*
- [ ] 11. Icônes : PNG 16×16 en 1 bit dans `images/` (une par stratagème),
  `fap_icon_assets="images"`, tableau `stratagem_icons[]` et `_Static_assert`
  dans `view.c`. — REQ-001-14 — verify : le build échoue si on retire une
  entrée du tableau ; contrôle visuel sur l'appareil.
- [ ] 12. Écran de manche définitif `view.c` : rangée de 5 icônes avec l'actif
  encadré au centre et les réussis barrés, nom, flèches pleines ou creuses,
  jauge, « M3 » et score, flash d'erreur de 120 ms en `ColorXor`. Remplace le
  rendu texte pour la phase `Playing`. — REQ-001-13, REQ-001-7 (flash) —
  verify : manuel, les quatre critères de REQ-001-13.
- [ ] 13. Autres écrans : titre avec record, « Manche N », fin de manche avec
  révélation des bonus toutes les 500 ms, pause sans contenu de jeu, game over
  avec « Nouveau record ! ». Supprime le rendu texte provisoire. — REQ-001-1, 3,
  12, 16, 19 (affichage) — verify : manuel, chaque critère d'affichage.
- [ ] 14. Pause sur perte de l'écran : `last_draw_tick` dans le callback de rendu,
  `game_pause` si plus de 250 ms sans rendu en phase active. — REQ-001-18 —
  verify : manuel, en ouvrant une session qFlipper en pleine manche. Si la GUI
  appelle quand même le callback d'un ViewPort masqué : barrer REQ-001-18 avec
  l'accord de l'utilisateur, et retirer le code.
- [ ] 15. Instrumentation de mesure derrière une constante de compilation
  (désactivée par défaut) : intervalle entre rendus, latence entre appui et
  rendu, tas libre au début et à la fin d'une partie. — REQ-001-24, 25 —
  verify : manuel, rendus ≤ 40 ms d'écart, latence ≤ 40 ms, tas identique
  après 5 manches.

## Clôture

- [ ] 16. Synchroniser la spec système : architecture (modules, timer), interfaces
  (touches, fichier de record), données (table, record), invariants ; retirer
  les questions ouvertes résolues ; passer cette spec en `implemented`. —
  verify : rapport de l'agent spec-keeper sans écart.
