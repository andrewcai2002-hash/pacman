/*
 * include/types.h
 * Définitions communes : constantes, types de base partagés par tous les modules.
 */

#ifndef TYPES_H
#define TYPES_H

/* ─── Dimensions de la fenêtre ─────────────────────────────────────────── */
#define WINDOW_WIDTH   560   /* 28 colonnes × 20 pixels                     */
#define WINDOW_HEIGHT  680   /* 31 lignes × 20 pixels (620) + 60px HUD      */

/* ─── Grille de jeu ─────────────────────────────────────────────────────── */
#define TILE_SIZE      20    /* Taille d'une case en pixels                  */
#define MAP_COLS       28    /* Nombre de colonnes                           */
#define MAP_ROWS       31    /* Nombre de lignes                             */
#define HUD_HEIGHT     60    /* Hauteur de la zone d'interface (score, vies) */

/* ─── Valeurs des cases de la carte ─────────────────────────────────────── */
#define TILE_EMPTY     0     /* Case vide (couloir)                         */
#define TILE_WALL      1     /* Mur                                         */
#define TILE_DOT       2     /* Petite pastille                             */
#define TILE_SUPER     3     /* Super pastille (power-up)                   */
#define TILE_SPAWN     4     /* Spawn du joueur (vide au chargement)        */
#define TILE_GHOST_HOUSE 5   /* Zone de départ des fantômes                 */

/* ─── Paramètres du joueur ───────────────────────────────────────────────── */
#define PLAYER_START_LIVES  3
#define PLAYER_DOT_SCORE    10
#define PLAYER_SUPER_SCORE  50
#define PLAYER_SPEED        1    /* Cases par tick                          */

/* ─── Paramètres des fantômes ────────────────────────────────────────────── */
#define GHOST_COUNT         4
#define GHOST_SCARED_TICKS  360  /* Durée du mode bleu: ~6 secondes (360 frames) */
#define GHOST_EXIT_DELAY_1  0    /* Blinky: sort immédiatement                   */
#define GHOST_EXIT_DELAY_2  120  /* Pinky: 2 secondes (120 frames)               */
#define GHOST_EXIT_DELAY_3  300  /* Inky: 5 secondes (300 frames)                */
#define GHOST_EXIT_DELAY_4  480  /* Clyde: 8 secondes (480 frames)               */
#define GHOST_EATEN_POINTS_1 200   /* 1er fantôme mangé                          */
#define GHOST_EATEN_POINTS_2 400   /* 2e fantôme mangé                           */
#define GHOST_EATEN_POINTS_3 800   /* 3e fantôme mangé                           */
#define GHOST_EATEN_POINTS_4 1600  /* 4e fantôme mangé                           */

/* ─── Framerate cible ────────────────────────────────────────────────────── */
#define TARGET_FPS          60   /* Frames de rendu par seconde              */
#define FRAME_DELAY_MS      (1000 / TARGET_FPS)
#define PLAYER_MOVE_DELAY   8    /* Pacman bouge 1 case toutes les 8 frames  */
#define GHOST_MOVE_DELAY    12   /* Fantômes bougent toutes les 12 frames    */

/* ─── États du jeu ───────────────────────────────────────────────────────── */
typedef enum {
    STATE_MENU,       /* Écran titre                                        */
    STATE_PLAYING,    /* Partie en cours                                    */
    STATE_WIN,        /* Le joueur a gagné                                  */
    STATE_GAME_OVER,  /* Plus de vies                                       */
    STATE_QUIT        /* Signal de fermeture                                */
} GameState;

/* ─── Structure position sur la grille ──────────────────────────────────── */
typedef struct {
    int x;   /* Colonne */
    int y;   /* Ligne   */
} Vector2;

/* ─── Directions ─────────────────────────────────────────────────────────── */
typedef enum {
    DIR_NONE  = 0,
    DIR_UP    = 1,
    DIR_DOWN  = 2,
    DIR_LEFT  = 3,
    DIR_RIGHT = 4
} Direction;

#endif /* TYPES_H */