# Pacman – Projet C avec SDL2

Jeu 2D de type Pacman développé en C (C99) avec la bibliothèque graphique SDL2, dans le cadre d'un projet pédagogique individuel.

---

## Structure du projet

```
pacman/
├── src/
│   ├── main.c        – Point d'entrée : initialisation SDL2, fenêtre, renderer
│   ├── game.c        – Boucle de jeu, gestion des états, mise à jour logique
│   ├── map.c         – Carte 2D, pastilles, collisions murs
│   ├── player.c      – Déplacements du joueur, collecte, score
│   ├── ghost.c       – IA des fantômes, mode peur
│   └── render.c      – Affichage carte, entités, HUD
├── include/
│   ├── types.h       – Constantes, types partagés (Vector2, Direction, GameState)
│   ├── game.h
│   ├── map.h
│   ├── player.h
│   ├── ghost.h
│   └── render.h
├── Makefile
└── README.md
```

---

## Prérequis

- GCC (ou Clang)
- SDL2 installée sur la machine

### Installation SDL2

**Ubuntu / Debian :**
```bash
sudo apt install libsdl2-dev
```

**macOS (Homebrew) :**
```bash
brew install sdl2
```

**Windows (MSYS2) :**
```bash
pacman -S mingw-w64-x86_64-SDL2
```

---

## Compilation et lancement

```bash
# Compiler
make

# Compiler et lancer directement
make run

# Nettoyer les fichiers compilés
make clean
```

---

## Contrôles

| Touche | Action |
|---|---|
| ↑ / Z / W | Déplacer vers le haut |
| ↓ / S | Déplacer vers le bas |
| ← / Q / A | Déplacer vers la gauche |
| → / D | Déplacer vers la droite |
| Entrée | Démarrer / Recommencer |
| Échap | Menu / Quitter |

---

## Fonctionnalités implémentées

- **Carte 2D** : labyrinthe 28×31 codé en tableau, murs, pastilles, super pastilles
- **Joueur** : déplacement fluide, collision murs, tunnel horizontal
- **Score** : pastilles (+10), super pastilles (+50), fantômes mangés (+200)
- **4 Fantômes** : IA de poursuite par distance de Manhattan, mode "peur" (bleu) activé par les super pastilles
- **Vies** : 3 vies, respawn après collision avec un fantôme
- **Victoire** : toutes les pastilles collectées → "YOU WIN!"
- **Game Over** : plus aucune vie → "GAME OVER"
- **Meilleur score** : sauvegardé automatiquement dans `highscore.txt`
- **Écran titre** : menu au démarrage, appuyer sur Entrée pour jouer

---

## Architecture – Boucle de jeu

```
Boucle principale (10 ticks/seconde)
│
├── 1. Événements SDL  (clavier, fermeture)
│       └─ player_set_direction()
│
├── 2. Mise à jour logique
│       ├─ player_update()     → déplacement + collecte pastilles
│       ├─ ghost_update() ×4  → IA + déplacement
│       └─ Collisions joueur/fantômes → respawn ou game over
│
└── 3. Rendu
        ├─ render_map()
        ├─ render_player()
        ├─ render_ghosts()
        ├─ render_hud()
        └─ SDL_RenderPresent()
```

---

## Pistes d'amélioration

- **Pathfinding A*** pour les fantômes (comportements individuels : Blinky, Pinky, Inky, Clyde)
- **SDL2_ttf** pour un affichage propre du score et des messages texte
- **SDL2_image** pour des sprites PNG (Pacman animé, fantômes)
- **Plusieurs niveaux** chargés depuis des fichiers `.txt`
- **Sons** avec SDL2_mixer (sirènes, collecte, mort)
- **Animation de Pacman** (gueule ouverte/fermée selon la direction)

---

## Auteur

Projet pédagogique – Développement C avec SDL2.
