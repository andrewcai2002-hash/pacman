/*
 * include/types.h
 * Constantes et types qu'on partage partout dans le projet.
 */

#ifndef TYPES_H
#define TYPES_H

/* Taille de la fenêtre */
#define WINDOW_WIDTH   560   /* 28 colonnes × 20 pixels */
#define WINDOW_HEIGHT  680   /* 31 lignes + 60px pour le score/vies */

/* Dimensions de la grille */
#define TILE_SIZE      20    /* Une case = 20 pixels */
#define MAP_COLS       28
#define MAP_ROWS       31
#define HUD_HEIGHT     60    /* Zone d'info en haut */

/* Types de tuiles */
#define TILE_EMPTY     0     /* Case vide */
#define TILE_WALL      1
#define TILE_DOT       2     /* Pastille normale */
#define TILE_SUPER     3     /* Super pastille */
#define TILE_SPAWN     4     /* Point de départ */
#define TILE_GHOST_HOUSE 5   /* La maison des fantômes */

/* Paramètres du joueur */
#define PLAYER_START_LIVES  3
#define PLAYER_DOT_SCORE    10
#define PLAYER_SUPER_SCORE  50
#define PLAYER_SPEED        1

/* Paramètres des fantômes */
#define GHOST_COUNT         4
#define GHOST_SCARED_TICKS  360  /* ~6 secondes en mode bleu */
#define GHOST_EXIT_DELAY_1  0    /* Blinky sort tout de suite */
#define GHOST_EXIT_DELAY_2  120  /* Pinky attend 2s */
#define GHOST_EXIT_DELAY_3  300  /* Inky attend 5s */
#define GHOST_EXIT_DELAY_4  480  /* Clyde attend 8s */
#define GHOST_EATEN_POINTS_1 200
#define GHOST_EATEN_POINTS_2 400
#define GHOST_EATEN_POINTS_3 800
#define GHOST_EATEN_POINTS_4 1600

/* Timing */
#define TARGET_FPS          60
#define FRAME_DELAY_MS      (1000 / TARGET_FPS)
#define PLAYER_MOVE_DELAY   8    /* Pacman avance tous les 8 frames */
#define GHOST_MOVE_DELAY    12

/* États du jeu */
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_WIN,
    STATE_GAME_OVER,
    STATE_QUIT
} GameState;

/* Position sur la grille */
typedef struct {
    int x;   /* Colonne */
    int y;   /* Ligne   */
} Vector2;

/* Directions de mouvement */
typedef enum {
    DIR_NONE  = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

#endif /* TYPES_H */