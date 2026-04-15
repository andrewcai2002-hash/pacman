/*
 * include/render.h
 * Affichage du jeu : la carte, les personnages, l'interface.
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

/* Couleurs des fantômes */
#define COLOR_GHOST_0  255,   0,   0, 255  /* Blinky, le rouge */
#define COLOR_GHOST_1  255, 184, 255, 255  /* Pinky, le rose */
#define COLOR_GHOST_2    0, 255, 255, 255  /* Inky, le cyan */
#define COLOR_GHOST_3  255, 184,  82, 255  /* Clyde, l'orange */
#define COLOR_SCARED_R   0,   0, 255, 255  /* Bleu quand ils ont peur */

/* Fonctions */

/* Efface l'ecran */
void render_clear(SDL_Renderer *renderer);

/* Dessine la carte */
void render_map(SDL_Renderer *renderer, const Map *map, int tick);

/* Dessine Pacman */
void render_player(SDL_Renderer *renderer, const Player *player);

/* Dessine les fantômes */
void render_ghosts(SDL_Renderer *renderer, const Ghost ghosts[GHOST_COUNT]);

/* Affiche le HUD (score, high score, niveau, vies) */
void render_hud(SDL_Renderer *renderer, const Player *player, int level, int high_score);

/* Affiche du texte au milieu (menu, game over, etc) */
void render_overlay_text(SDL_Renderer *renderer, const char *label);

/* Affiche tout à l'écran */
void render_present(SDL_Renderer *renderer);

#endif /* RENDER_H */