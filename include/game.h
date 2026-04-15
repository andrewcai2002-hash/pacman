/*
 * include/game.h
 * La structure du jeu et les fonctions principales.
 */

#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "types.h"
#include "map.h"
#include "player.h"
#include "ghost.h"

/* La structure du jeu */
typedef struct {
    GameState  state;               /* État courant (menu, playing, …)      */
    Map        map;                 /* Carte de jeu                         */
    Player     player;              /* Joueur                               */
    Ghost      ghosts[GHOST_COUNT]; /* Fantômes                             */
    int        high_score;          /* Meilleur score (chargé depuis fichier)*/
    int        tick;                /* Compteur de ticks de logique          */
    int        level;               /* Numéro de niveau courant (commence à 1) */
    int        ghosts_eaten_count;  /* Nombre de fantômes mangés en mode bleu */
} Game;

/* Définitions du high score */
#define HIGHSCORE_FILE "highscore.txt"

/* Fonctions */

/* Initialise le jeu avec la map, le joueur et les fantômes */
 /*
 * @param game      Pointeur vers la structure Game
 * @param renderer  Renderer SDL2
 */
void game_init(Game *game, SDL_Renderer *renderer);

/* Lance la boucle du jeu */
 /*
 * @param game      Structure du jeu
 * @param window    Fenêtre SDL2
 * @param renderer  Renderer SDL2
 */
void game_run(Game *game, SDL_Window *window, SDL_Renderer *renderer);

/* Réinitialise la carte, le joueur et les fantômes */
 /*
 * @param game  Structure du jeu
 */
void game_reset(Game *game);

/* Sauvegarde le high score et libère les ressources */
 /*
 * @param game  Structure du jeu
 */
void game_quit(Game *game);

#endif /* GAME_H */