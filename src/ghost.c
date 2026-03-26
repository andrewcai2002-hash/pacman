/*
 * src/ghost.c
 * Implémentation du module fantôme.
 *
 * La maison des fantômes (TILE_GHOST_HOUSE) est une zone spéciale :
 *   - les fantômes peuvent se déplacer dedans (logique séparée)
 *   - une fois sortis ils n'y rentrent plus
 *
 * Sortie : chaque fantôme remonte vers la porte (EXIT_X=13, EXIT_Y=11)
 * avec un délai décalé pour ne pas sortir tous en même temps.
 */

#include <stdlib.h>
#include <time.h>
#include "../include/ghost.h"
#include "../include/types.h"

/* ─── Porte de sortie de la maison ──────────────────────────────────────── */
#define EXIT_X  13
#define EXIT_Y  12

/* ─── Spawn des 4 fantômes à l'intérieur de la maison ───────────────────── */
static const int SPAWN_X[GHOST_COUNT] = {13, 13, 12, 14};
static const int SPAWN_Y[GHOST_COUNT] = {13, 14, 14, 14};

/* Délai de sortie de la maison au début du jeu (en frames)
 * Blinky (0): immédiat, Pinky (1): 2s, Inky (2): 5s, Clyde (3): 8s */
static const int EXIT_DELAY[GHOST_COUNT] = {0, 120, 300, 480};

/* Délai de réapparition après être mangé (en frames)
 * 1er fantôme : 2s, 2ème : 4s, 3ème : 6s, 4ème : 8s */
static const int RESPAWN_DELAY[GHOST_COUNT] = {120, 240, 360, 480};

/* ─── Utilitaires ────────────────────────────────────────────────────────── */
static const Direction ALL_DIRS[4] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};

static Direction opposite(Direction d)
{
    switch (d) {
        case DIR_UP:    return DIR_DOWN;
        case DIR_DOWN:  return DIR_UP;
        case DIR_LEFT:  return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        default:        return DIR_NONE;
    }
}

static Vector2 dir_to_vec(Direction d)
{
    Vector2 v = {0, 0};
    switch (d) {
        case DIR_UP:    v.y = -1; break;
        case DIR_DOWN:  v.y =  1; break;
        case DIR_LEFT:  v.x = -1; break;
        case DIR_RIGHT: v.x =  1; break;
        default: break;
    }
    return v;
}

static int manhattan(Vector2 a, Vector2 b)
{
    int dx = a.x - b.x; if (dx < 0) dx = -dx;
    int dy = a.y - b.y; if (dy < 0) dy = -dy;
    return dx + dy;
}

/* Vérifie si une case est franchissable pour un fantôme
 * (walkable normal OU ghost_house si pas encore sorti) */
static int ghost_can_walk(const Map *map, int x, int y, int exited)
{
    int t = map_get_tile(map, x, y);
    if (t == TILE_WALL) return 0;
    if (t == TILE_GHOST_HOUSE && exited) return 0; /* Sorti = interdit de rentrer */
    return 1;
}

/* ─── Implémentation publique ────────────────────────────────────────────── */

void ghost_init(Ghost ghosts[GHOST_COUNT])
{
    int i;
    srand((unsigned int)time(NULL));

    for (i = 0; i < GHOST_COUNT; i++) {
        ghosts[i].pos.x        = SPAWN_X[i];
        ghosts[i].pos.y        = SPAWN_Y[i];
        ghosts[i].spawn.x      = SPAWN_X[i];
        ghosts[i].spawn.y      = SPAWN_Y[i];
        ghosts[i].dir          = DIR_UP;
        ghosts[i].state        = GHOST_NORMAL;
        ghosts[i].scared_timer = 0;
        ghosts[i].exited       = 0;
        ghosts[i].dead_timer   = 0;
        ghosts[i].respawn_delay= 0;
        ghosts[i].exit_delay   = EXIT_DELAY[i];  /* Délai avant de pouvoir sortir */
        ghosts[i].move_timer   = 0;
        ghosts[i].color_id     = i;  /* Index 0=rouge, 1=cyan, 2=rose, 3=orange */
    }
}

void ghost_update(Ghost *ghost, const Map *map, Vector2 player_pos)
{
    int i;

    /* Gestion du délai de réapparition après être mangé */
    if (ghost->state == GHOST_DEAD) {
        ghost->dead_timer--;
        if (ghost->dead_timer <= 0) {
            /* Le délai est écoulé, passer en mode sortie */
            ghost->state = GHOST_NORMAL;
            ghost->exited = 0;  /* Réinitialiser pour pouvoir sortir */
            ghost->exit_delay = 0;  /* Peut sortir immédiatement après réapparition */
        }
        return;  /* Rester dans la maison durant le délai */
    }
    
    /* Gestion du délai initial de sortie au début du jeu */
    if (ghost->exit_delay > 0) {
        ghost->exit_delay--;
        if (ghost->exit_delay <= 0) {
            ghost->exited = 0;  /* Permet à la phase de sortie de commencer */
        }
        return;  /* Rester dans la maison durant le délai initial */
    }

    /* Décrémenter le timer de peur */
    if (ghost->state == GHOST_SCARED) {
        ghost->scared_timer--;
        if (ghost->scared_timer <= 0)
            ghost->state = GHOST_NORMAL;
    }

    /* Vitesse : normal vs peur */
    int speed = (ghost->state == GHOST_SCARED) ? GHOST_MOVE_DELAY + 6 : GHOST_MOVE_DELAY;
    ghost->move_timer++;
    if (ghost->move_timer < speed)
        return;
    ghost->move_timer = 0;

    /* ── Phase de sortie de la maison ── */
    if (!ghost->exited) {
        /* Remonter vers EXIT_Y puis se centrer sur EXIT_X
         * Vérifier à chaque pas qu'il n'y a pas de mur */
        
        if (ghost->exit_delay <= 0) {
            /* Centrer sur EXIT_X d'abord */
            if (ghost->pos.x < EXIT_X && ghost_can_walk(map, ghost->pos.x + 1, ghost->pos.y, 0)) {
                ghost->pos.x++;
            } else if (ghost->pos.x > EXIT_X && ghost_can_walk(map, ghost->pos.x - 1, ghost->pos.y, 0)) {
                ghost->pos.x--;
            } else if (ghost->pos.x == EXIT_X && ghost->pos.y > EXIT_Y && ghost_can_walk(map, ghost->pos.x, ghost->pos.y - 1, 0)) {
                ghost->pos.y--;
            } else if (ghost->pos.x == EXIT_X && ghost->pos.y == EXIT_Y) {
                /* Arrivé à la sortie */
                ghost->exited = 1;
                ghost->dir    = DIR_LEFT;
            }
        }
        
        return;
    }

    /* ── IA : choisir la meilleure direction ── */
    Direction best_dir = DIR_NONE;
    int best_score = -1;
    Vector2 best_pos;
    best_pos.x = ghost->pos.x;
    best_pos.y = ghost->pos.y;

    for (i = 0; i < 4; i++) {
        Direction d = ALL_DIRS[i];

        if (d == opposite(ghost->dir))
            continue;

        Vector2 dv = dir_to_vec(d);
        int nx = ghost->pos.x + dv.x;
        int ny = ghost->pos.y + dv.y;

        /* Tunnel horizontal */
        if (nx < 0)         nx = MAP_COLS - 1;
        if (nx >= MAP_COLS) nx = 0;

        if (!ghost_can_walk(map, nx, ny, ghost->exited))
            continue;

        Vector2 candidate = {nx, ny};
        int dist = manhattan(candidate, player_pos);

        /* Part d'aléatoire pour casser les boucles */
        if (rand() % 5 == 0)
            dist += (rand() % 5) - 2;

        if (ghost->state == GHOST_SCARED) {
            if (best_dir == DIR_NONE || dist > best_score) {
                best_score = dist;
                best_dir   = d;
                best_pos   = candidate;
            }
        } else {
            if (best_dir == DIR_NONE || dist < best_score) {
                best_score = dist;
                best_dir   = d;
                best_pos   = candidate;
            }
        }
    }

    /* Fallback demi-tour si bloqué */
    if (best_dir == DIR_NONE) {
        Direction back = opposite(ghost->dir);
        Vector2 dv = dir_to_vec(back);
        int nx = ghost->pos.x + dv.x;
        int ny = ghost->pos.y + dv.y;
        if (ghost_can_walk(map, nx, ny, ghost->exited)) {
            best_dir   = back;
            best_pos.x = nx;
            best_pos.y = ny;
        }
    }

    if (best_dir != DIR_NONE) {
        ghost->dir = best_dir;
        ghost->pos = best_pos;
    }
}

void ghost_scare_all(Ghost ghosts[GHOST_COUNT])
{
    int i;
    for (i = 0; i < GHOST_COUNT; i++) {
        if (ghosts[i].state != GHOST_DEAD) {
            ghosts[i].state        = GHOST_SCARED;
            ghosts[i].scared_timer = GHOST_SCARED_TICKS;
        }
    }
}

int ghost_collides_with_player(const Ghost *ghost, Vector2 player_pos)
{
    return (ghost->pos.x == player_pos.x && ghost->pos.y == player_pos.y);
}

int ghost_get_respawn_delay(int ghost_index)
{
    if (ghost_index < 0 || ghost_index >= GHOST_COUNT)
        return RESPAWN_DELAY[0];
    return RESPAWN_DELAY[ghost_index];
}