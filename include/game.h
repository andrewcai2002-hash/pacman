/*
 * include/game.h
 * Interface du module principal du jeu : structure Game, boucle, états.
 */

#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "types.h"
#include "map.h"
#include "player.h"
#include "ghost.h"

/* ─── Structure principale du jeu ───────────────────────────────────────── */
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

/* ─── Fichier de sauvegarde du high score ────────────────────────────────── */
#define HIGHSCORE_FILE "highscore.txt"

/* ─── Fonctions publiques ────────────────────────────────────────────────── */

/**
 * game_init  –  Initialise la structure Game et charge les données.
 * @param game      Pointeur vers la structure Game.
 * @param renderer  Renderer SDL2 (conservé pour les transitions futures).
 */
void game_init(Game *game, SDL_Renderer *renderer);

/**
 * game_run  –  Lance et maintient la boucle principale du jeu.
 * @param game      Pointeur vers la structure Game.
 * @param window    Fenêtre SDL2.
 * @param renderer  Renderer SDL2.
 */
void game_run(Game *game, SDL_Window *window, SDL_Renderer *renderer);

/**
 * game_reset  –  Remet le jeu à zéro (nouveau niveau ou après game over).
 * @param game  Pointeur vers la structure Game.
 */
void game_reset(Game *game);

/**
 * game_quit  –  Libère les ressources internes (sauvegarde high score).
 * @param game  Pointeur vers la structure Game.
 */
void game_quit(Game *game);

#endif /* GAME_H */