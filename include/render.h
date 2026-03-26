/*
 * include/render.h
 * Interface du module rendu : affichage carte, joueur, fantômes, HUD.
 */

#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "types.h"
#include "map.h"
#include "player.h"
#include "ghost.h"

/* ─── Couleurs principales ───────────────────────────────────────────────── */
#define COLOR_BG_R      0,   0,   0, 255   /* Fond noir                    */
#define COLOR_WALL_R   33,  33, 222, 255   /* Mur bleu Pacman              */
#define COLOR_DOT_R   255, 255, 184, 255   /* Pastille crème               */
#define COLOR_SUPER_R 255, 184,   0, 255   /* Super pastille orange        */
#define COLOR_PLAYER_R 255, 255,   0, 255  /* Joueur jaune                 */
#define COLOR_HUD_R    50,  50,  50, 255   /* Fond HUD                     */

/* Couleurs des 4 fantômes */
#define COLOR_GHOST_0  255,   0,   0, 255  /* Rouge  – Blinky              */
#define COLOR_GHOST_1  255, 184, 255, 255  /* Rose   – Pinky               */
#define COLOR_GHOST_2    0, 255, 255, 255  /* Cyan   – Inky                */
#define COLOR_GHOST_3  255, 184,  82, 255  /* Orange – Clyde               */
#define COLOR_SCARED_R   0,   0, 255, 255  /* Fantôme effrayé bleu         */

/* ─── Fonctions publiques ────────────────────────────────────────────────── */

/**
 * render_clear  –  Efface l'écran avec la couleur de fond.
 */
void render_clear(SDL_Renderer *renderer);

/**
 * render_map  –  Dessine la carte (murs, pastilles, cases vides).
 * @param tick  Tick courant du jeu, utilisé pour le clignotement des super pastilles.
 */
void render_map(SDL_Renderer *renderer, const Map *map, int tick);

/**
 * render_player  –  Dessine le joueur (cercle jaune rempli).
 */
void render_player(SDL_Renderer *renderer, const Player *player);

/**
 * render_ghosts  –  Dessine tous les fantômes.
 */
void render_ghosts(SDL_Renderer *renderer, const Ghost ghosts[GHOST_COUNT]);

/**
 * render_hud  –  Affiche le score, le high score, le niveau et les vies.
 * @param level       Numéro de niveau courant.
 * @param high_score  Meilleur score global.
 */
void render_hud(SDL_Renderer *renderer, const Player *player, int level, int high_score);

/**
 * render_overlay_text  –  Affiche un message centré (menu, game over, win).
 *                          Implémentation minimaliste avec des rectangles.
 */
void render_overlay_text(SDL_Renderer *renderer, const char *label);

/**
 * render_present  –  Présente le rendu à l'écran (swap buffer).
 */
void render_present(SDL_Renderer *renderer);

#endif /* RENDER_H */