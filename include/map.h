/*
 * include/map.h
 * Interface du module carte : chargement, accès aux cases, gestion des pastilles.
 */

#ifndef MAP_H
#define MAP_H

#include "types.h"

/* ─── Structure principale de la carte ──────────────────────────────────── */
typedef struct {
    int  tiles[MAP_ROWS][MAP_COLS]; /* Grille de cases                      */
    int  dot_count;                 /* Pastilles restantes (dot + super)     */
    int  total_dots;                /* Total au début du niveau              */
} Map;

/* ─── Fonctions publiques ────────────────────────────────────────────────── */

/**
 * map_load  –  Initialise la carte avec la grille codée en dur (niveau 1).
 * @param map  Pointeur vers la structure Map à remplir.
 */
void map_load(Map *map);

/**
 * map_get_tile  –  Retourne la valeur d'une case (ou TILE_WALL si hors limites).
 * @param map  Carte.
 * @param x    Colonne.
 * @param y    Ligne.
 * @return     Valeur de la case.
 */
int  map_get_tile(const Map *map, int x, int y);

/**
 * map_set_tile  –  Modifie la valeur d'une case.
 * @param map    Carte.
 * @param x      Colonne.
 * @param y      Ligne.
 * @param value  Nouvelle valeur.
 */
void map_set_tile(Map *map, int x, int y, int value);

/**
 * map_is_walkable  –  Indique si une case est franchissable par une entité.
 * @param map  Carte.
 * @param x    Colonne.
 * @param y    Ligne.
 * @return     1 si franchissable, 0 sinon.
 */
int  map_is_walkable(const Map *map, int x, int y);

/**
 * map_count_dots  –  Recompte les pastilles restantes (utile après respawn).
 * @param map  Carte.
 * @return     Nombre de pastilles restantes.
 */
int  map_count_dots(const Map *map);

#endif /* MAP_H */
