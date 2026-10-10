---
status: draft
updated: 2026-10-10
---

# 001 — Règles du jeu : journal des décisions

Entretien grill-me sur les règles de Stratagem Hero version Flipper.
Source des questions : section « Questions ouvertes » de `docs/spec/README.md`.

## D1 — Fidélité à l'original
- Decision: fidèle mais adapté. On reprend les mécaniques de Stratagem Hero (manches, chrono qui se recharge, bonus de manche, de temps et de manche parfaite) et les vrais noms de stratagèmes, avec des valeurs réglées pour l'écran 128×64 et les boutons physiques.
- Why: reconnaissable pour un joueur de Helldivers, sans dépendre de valeurs exactes qu'on ne connaît pas avec certitude.
- Rejected: fidèle au chiffre près (valeurs d'origine non vérifiées) ; libre inspiration (perd l'intérêt de la référence).
- Decided by: recommendation accepted

## D2 — Chrono de manche
- Decision: une jauge par manche, à 10 s au départ ; +1,5 s par stratagème réussi, plafonnée à 10 s ; remplie à nouveau au début de chaque manche. Jauge à zéro → game over.
- Why: proche de l'original, la vitesse rapporte du temps et crée la tension.
- Rejected: chrono par stratagème (perd la tension d'enchaînement) ; valeurs réglables (écran d'options en plus, remet en cause l'ADR-0002).
- Decided by: recommendation accepted
- Note: une éventuelle accélération au fil des manches est traitée avec la progression.

## D3 — Erreur de saisie
- Decision: la séquence en cours repart du début, avec un retour (vibreur et flash à l'écran). Pas de pénalité de temps. Toute erreur dans la manche annule le bonus de manche parfaite.
- Why: fidèle à l'original ; le temps perdu à recommencer suffit comme sanction.
- Rejected: pénalité de temps en plus (double sanction) ; erreur ignorée (on peut marteler les touches).
- Decided by: recommendation accepted

## D4 — Événements de touche pris en compte
- Decision: pendant une manche, une flèche compte dès l'appui (`InputTypePress`) ; `InputTypeLong` et `InputTypeRepeat` sont ignorés. Les touches de navigation (OK, Back) restent sur `InputTypeShort`.
- Why: un jeu de vitesse ne doit pas attendre le relâchement ; sans repeat, maintenir une touche ne produit pas d'erreur involontaire.
- Rejected: Short pour les flèches (latence au relâchement) ; Press + repeat (erreurs involontaires).
- Decided by: recommendation accepted
- Consequence: le squelette actuel filtre sur `InputTypeShort`, il faudra le changer pour les flèches.

## D5 — Barème
- Decision:
  - stratagème réussi : 5 pts × longueur de sa séquence ;
  - fin de manche : bonus de manche 75 + 25 × (n° de manche − 1) ;
  - bonus de temps : 10 pts par seconde restante, au dixième près (temps restant en ms / 100, arrondi vers le bas), 100 max ;
  - manche sans aucune erreur : +100.
- Why: mêmes types de bonus que l'original (D1), qui récompensent la vitesse et la précision.
- Rejected: 1 point par stratagème (plus aucun intérêt à aller vite ni à être parfait).
- Decided by: recommendation accepted
- Note: valeurs inspirées de l'original sans vérification ; elles pourront être ajustées après des tests sur l'appareil.

## D6 — Taille des manches et progression
- Decision: la manche n contient min(6 + n, 16) stratagèmes, tirés au hasard dans la table sans répéter deux fois de suite le même. Le chrono (D2) ne change pas d'une manche à l'autre. La partie ne s'arrête qu'au game over.
- Why: la difficulté monte progressivement par la longueur des manches, sans rendre le chrono injouable.
- Rejected: taille fixe avec recharge qui baisse ; les deux à la fois (la difficulté monte trop vite).
- Decided by: recommendation accepted
- Consequence: à partir de la manche 10, la difficulté ne monte plus ; le score départage alors les joueurs.

## D7 — Liste des stratagèmes
- Decision: une sélection d'environ 20 stratagèmes connus, avec des séquences de 3 à 8 flèches, dans une table constante facile à compléter. Claude rédige la liste ; l'utilisateur valide les séquences avant l'implémentation.
- Why: assez de variété pour des manches de 16, et peu de séquences à vérifier.
- Rejected: liste complète (plus de 60 séquences à vérifier, et le jeu en ajoute régulièrement) ; liste fournie par l'utilisateur.
- Decided by: recommendation accepted
- Open: validation des séquences par l'utilisateur (bloque l'implémentation de la table, pas la spec).

## D8 — Affichage d'un stratagème et icônes
- Decision: icônes dès la v1. Chaque stratagème a une icône 1 bit de 16×16 (PNG dans `images/`, compilée par `fap_icon_assets`), affichée avec son nom, sa séquence en flèches dessinées (pleines quand elles sont déjà saisies, creuses pour les suivantes) et la jauge du chrono. Claude dessine les icônes en pixel art ; l'utilisateur retouche celles qui ne lui conviennent pas.
- Why: plus proche de l'original ; 16×16 laisse de la place au reste sur 64 px de haut.
- Rejected: nom et flèches sans icône (recommandé, mais l'utilisateur préfère les icônes) ; icônes dessinées par l'utilisateur ; 24×24 (écran trop serré).
- Decided by: user (icônes), puis recommendation accepted (Claude, 16×16)
- Consequence: les icônes sont redessinées, pas copiées du jeu (non-objectif de la spec système) ; `application.fam` gagne `fap_icon_assets="images"`.

## D9 — Aperçu des stratagèmes et disposition de l'écran de manche
- Decision: une rangée d'icônes 16×16 en haut, toutes de la même taille. L'actif, encadré, est au centre ; à sa gauche, les 2 derniers réussis de la manche (barrés) ; à sa droite, les 2 suivants. Les emplacements sans stratagème restent vides (début et fin de manche). En dessous : le nom de l'actif centré, puis ses flèches ; en bas : la jauge du chrono, la manche (« M3 ») et le score.
- Why: choix de l'utilisateur. On voit à la fois la progression et ce qui arrive, comme dans l'original.
- Rejected: aucun aperçu (recommandé, rejeté par l'utilisateur) ; actif à gauche + 4 ou 5 suivants ; compteur « 3/9 ».
- Decided by: user

## D10 — Retours physiques
- Decision: tout passe par les séquences du service de notification (`notification_message`). Erreur : vibration courte et LED rouge. Stratagème réussi : LED verte brève. Fin de manche et game over : une séquence dédiée (son et vibration). Le son et la vibration respectent les réglages du Flipper (mode silencieux, volume) ; l'app n'a pas de réglage propre.
- Why: retour clair sur l'erreur et la réussite sans aller contre les préférences de l'utilisateur, et sans écran d'options (ADR-0002).
- Rejected: vibreur seul (pas de retour de réussite) ; sons forcés (contournent le mode silencieux).
- Decided by: recommendation accepted

## D11 — Meilleur score
- Decision: un seul record (score et manche atteinte), affiché sur l'écran titre. Il est écrit à l'écran de game over, seulement s'il est battu, dans `APP_DATA_PATH("best.fff")` (`/ext/apps_data/flipper_diver/`) au format FlipperFormat (`Filetype`, `Version: 1`, `Score`, `Round`). Fichier absent, illisible ou de version inconnue : record = 0, sans message d'erreur bloquant ; il sera réécrit au prochain record.
- Why: motive à rejouer avec un seul fichier minuscule ; FlipperFormat est le format standard des apps et gère la version.
- Rejected: top 5 (écran en plus, pas de nom saisissable facilement) ; aucune persistance.
- Decided by: recommendation accepted (le chemin et le format ont été choisis par Claude, car c'est un détail d'implémentation)

## D12 — Pause et sortie en cours de partie
- Decision: Back pendant une manche met en pause. Le chrono s'arrête et l'écran de pause cache le stratagème et les icônes. OK reprend, Back quitte l'app. Quitter en cours de partie n'enregistre pas de record, puisque la partie n'est pas finie. Si l'app perd l'écran (verrouillage, notification système), elle se met aussi en pause.
- Why: un appui accidentel ne coûte pas la partie, et la pause ne permet pas de lire les séquences à l'avance.
- Rejected: pause = game over (punit un appui accidentel) ; pas de pause.
- Decided by: recommendation accepted
- Open (design): l'API publique ne semble pas notifier simplement une perte d'écran pour un ViewPort ; vérifier la faisabilité au design, sinon retirer ce point avec l'accord de l'utilisateur.

## D13 — Fin de manche
- Decision: l'écran de fin de manche affiche l'un après l'autre le bonus de manche, le bonus de temps, le bonus parfait (seulement s'il est obtenu) et le total. La manche suivante démarre au bout de 3 s, ou tout de suite avec OK. Back met en pause, comme pendant une manche.
- Why: comme l'original, on comprend d'où viennent les points sans casser le rythme.
- Rejected: OK obligatoire (casse le rythme) ; total seul (on ne voit pas d'où viennent les points).
- Decided by: recommendation accepted

## D14 — Début de manche et game over
- Decision: avant chaque manche, « Manche N » s'affiche 1,5 s, chrono arrêté ; les flèches pressées pendant ce temps sont ignorées. Le game over affiche le score, la manche atteinte, le record et « Nouveau record ! » s'il est battu. OK ramène au titre, Back quitte l'app.
- Why: un temps pour se préparer sans compte à rebours pénible ; le game over résume la partie.
- Rejected: départ immédiat (pas de répit) ; compte à rebours 3-2-1 (trop d'attente cumulée avec D13).
- Decided by: recommendation accepted

## D15 — Stratégie de test
- Decision: les règles (séquences, chrono, barème, progression, tirage) vivent dans un module C pur, sans dépendance à Furi, piloté par des événements et un temps et un aléa injectés. Des tests unitaires le compilent avec le gcc du PC sous ASan et UBSan, lancés par `devenv test`. L'app Flipper ne garde que l'affichage, les entrées, les notifications et le stockage.
- Why: tests rapides et reproductibles sans appareil ; non-régression du barème et du chrono.
- Rejected: tests sur l'appareil seulement (pas de non-régression) ; framework de tests unitaires du firmware (lourd avec ufbt, demande un Flipper branché).
- Decided by: recommendation accepted
- Consequence: ufbt compile par défaut tous les `.c` du dossier de l'app. Les tests doivent être exclus du FAP (champ `sources` de `application.fam` ou dossier hors de sa portée), à régler au design.

## D16 — Budgets de performance
- Decision: pendant une manche, l'écran se rafraîchit au moins à 25 Hz pour une jauge fluide. Entre un appui et son affichage, au plus 40 ms. Aucune allocation après l'initialisation (invariant de la spec système).
- Why: un jeu de réflexe doit afficher la saisie sans retard perceptible.
- Rejected: aucun (valeurs à confirmer sur l'appareil).
- Decided by: delegated (Claude), car ce n'était pas une question posée

## Questions ouvertes

- Séquences des ~20 stratagèmes : Claude rédige la liste, l'utilisateur la valide (D7). Bloque l'implémentation de la table.
- Pause quand l'app perd l'écran : faisabilité avec l'API publique (D12). Bloque une exigence ; à trancher au design.
- Exclusion des tests du build FAP (D15). À trancher au design.
- Valeurs du barème et du chrono : à ajuster après des tests sur l'appareil (D2, D5). Ne bloque rien.
