/*
 * src/map.c
 * La carte du jeu Pacman (original de 1980).
 *
 * 0 = vide, 1 = mur, 2 = pastille, 3 = super, 4 = spawn, 5 = maison des fantômes
 */

#include <stdio.h>
#include "../include/map.h"

/* Carte Pacman 28×31 (Namco 1980) */
static const int DEFAULT_MAP[MAP_ROWS][MAP_COLS] = {
/* col:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 */
/*  0 */ {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*  1 */ {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
/*  2 */ {1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
/*  3 */ {1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 3, 1},
/*  4 */ {1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
/*  5 */ {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
/*  6 */ {1, 2, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1},
/*  7 */ {1, 2, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1},
/*  8 */ {1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 1},
/*  9 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 10 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 11 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 12 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 5, 5, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 13 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 5, 5, 5, 5, 5, 5, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 14 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 5, 5, 5, 5, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 15 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 5, 5, 5, 5, 5, 5, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 16 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 17 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 18 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 19 */ {1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1},
/* 20 */ {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
/* 21 */ {1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
/* 22 */ {1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
/* 23 */ {1, 3, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 3, 1},
/* 24 */ {1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1},
/* 25 */ {1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1},
/* 26 */ {1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 1},
/* 27 */ {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
/* 28 */ {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
/* 29 */ {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
/* 30 */ {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

/* ─── Implémentation ─────────────────────────────────────────────────────── */

/* Charge la carte et compte les pastilles */
void map_load(Map *map)
{
    int r, c;
    map->dot_count = 0; /* Réinitialiser le compteur */

    /* Boucler sur toute la grille */
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            int v = DEFAULT_MAP[r][c];
            
            /**
             * Convertir TILE_SPAWN en TILE_EMPTY
             * (le spawn n'est que du marquage, pas une case spéciale visible)
             */
            map->tiles[r][c] = (v == TILE_SPAWN) ? TILE_EMPTY : v;
            
            /* Compter les pastilles (dots et super) */
            if (v == TILE_DOT || v == TILE_SUPER)
                map->dot_count++;
        }
    }
    /* Garder une copie du total initial pour réinitialisation */
    map->total_dots = map->dot_count;
}

/* Retourne le type d'une case (TILE_WALL si hors limites) */
int map_get_tile(const Map *map, int x, int y)
{
    /* Traiter les positions hors limites comme des murs */
    if (x < 0 || x >= MAP_COLS || y < 0 || y >= MAP_ROWS)
        return TILE_WALL;
    return map->tiles[y][x];
}

/**
 * map_set_tile  –  Modifie le type d'une case
 * 
 * Utilisée pour:
 * - Enlever une pastille quand le joueur la collecte
 * - Modifier l'état de la carte en jeu
 * 
 * Accès sécurisé: ne rien faire si hors limites.
 * 
 * @param map    Pointeur vers la carte
 * @param x      Colonne
 * @param y      Ligne
 * @param value  Nouvelle valeur pour la case
 */
void map_set_tile(Map *map, int x, int y, int value)
{
    /* Vérifier que les coordonnées sont valides */
    if (x < 0 || x >= MAP_COLS || y < 0 || y >= MAP_ROWS)
        return; /* Ignorer silencieusement hors limites */
    map->tiles[y][x] = value;
}

/**
 * map_is_walkable  –  Teste si une case est franchissable
 * 
 * Une case est marchable si elle n'est pas un mur.
 * Les créatures (joueur, fantômes) peuvent se déplacer sur:
 * - Cases vides (TILE_EMPTY)
 * - Pastilles (TILE_DOT, TILE_SUPER) 
 * - Zone de fantômes (TILE_GHOST_HOUSE) [à voir selon logique]
 * 
 * @param map  Pointeur vers la carte
 * @param x    Colonne
 * @param y    Ligne
 * @return     1 si marchable (pas de mur), 0 sinon
 */
int map_is_walkable(const Map *map, int x, int y)
{
    int t = map_get_tile(map, x, y);
    return (t != TILE_WALL); /* Tout sauf les murs est marchable */
}

/**
 * map_count_dots  –  Recompte les pastilles restantes sur la carte
 * 
 * Utile après un chargement pour vérifier la cohérence.
 * Boucle sur toute la carte et compte les TILE_DOT et TILE_SUPER.
 * 
 * @param map  Pointeur vers la carte
 * @return     Nombre de pastilles restantes
 */
int map_count_dots(const Map *map)
{
    int count = 0, r, c;
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            if (map->tiles[r][c] == TILE_DOT || map->tiles[r][c] == TILE_SUPER)
                count++;
        }
    }
    return count;
}

int map_load_from_file(Map *map, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        map_load(map);
        return 0;
    }

    int r, c;
    char line[MAP_COLS + 4];
    map->dot_count = 0;

    for (r = 0; r < MAP_ROWS; r++) {
        if (!fgets(line, sizeof(line), f)) {
            fclose(f);
            map_load(map);
            return 0;
        }
        for (c = 0; c < MAP_COLS; c++) {
            int v = line[c] - '0';
            if (v < 0 || v > 5) v = TILE_EMPTY;
            map->tiles[r][c] = (v == TILE_SPAWN) ? TILE_EMPTY : v;
            if (v == TILE_DOT || v == TILE_SUPER)
                map->dot_count++;
        }
    }
    map->total_dots = map->dot_count;
    fclose(f);
    return 1;
}