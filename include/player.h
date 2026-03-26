/*
 * include/player.h
 * Interface du module joueur : structure, déplacements, collecte, score.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "map.h"

/* ─── Structure du joueur ────────────────────────────────────────────────── */
typedef struct {
    Vector2   pos;          /* Position actuelle sur la grille              */
    Vector2   spawn;        /* Position de réapparition                     */
    Direction dir;          /* Direction de déplacement demandée            */
    Direction next_dir;     /* Prochaine direction (mémorisée)              */
    int       lives;        /* Vies restantes                               */
    int       score;        /* Score courant                                */
    int       move_timer;   /* Compteur pour ralentir le déplacement        */
    int       anim_frame;   /* 0 = bouche ouverte, 1 = bouche fermée        */
    int       anim_timer;   /* Compteur pour alterner les frames            */
} Player;

/* ─── Fonctions publiques ────────────────────────────────────────────────── */

/**
 * player_init  –  Initialise le joueur à sa position de départ.
 * @param player  Pointeur vers le joueur.
 * @param spawn_x Colonne de départ.
 * @param spawn_y Ligne de départ.
 */
void player_init(Player *player, int spawn_x, int spawn_y);

/**
 * player_set_direction  –  Mémorise la prochaine direction souhaitée.
 * @param player  Pointeur vers le joueur.
 * @param dir     Direction demandée.
 */
void player_set_direction(Player *player, Direction dir);

/**
 * player_update  –  Déplace le joueur et gère la collecte de pastilles.
 * @param player  Pointeur vers le joueur.
 * @param map     Carte pour la vérification des collisions.
 * @return        Valeur de la case collectée (TILE_DOT, TILE_SUPER ou TILE_EMPTY).
 */
int  player_update(Player *player, Map *map);

/**
 * player_respawn  –  Remet le joueur à sa position de spawn, perd une vie.
 * @param player  Pointeur vers le joueur.
 */
void player_respawn(Player *player);

#endif /* PLAYER_H */