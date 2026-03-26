/*
 * include/ghost.h
 * Interface du module fantôme : structure, IA de déplacement, états.
 */

#ifndef GHOST_H
#define GHOST_H

#include "types.h"
#include "map.h"

/* ─── États d'un fantôme ─────────────────────────────────────────────────── */
typedef enum {
    GHOST_NORMAL = 0,   /* Chasse le joueur / patrouille                   */
    GHOST_SCARED,       /* Fuit (super pastille activée)                   */
    GHOST_DEAD          /* Retourne à la maison                            */
} GhostState;

/* ─── Délai de réapparition après être mangé (en frames)
 * 1er fantôme : 2s, 2ème : 4s, 3ème : 6s, 4ème : 8s */
#define RESPAWN_DELAY_1  120   /* 2 secondes (120 frames)  */
#define RESPAWN_DELAY_2  240   /* 4 secondes (240 frames)  */
#define RESPAWN_DELAY_3  360   /* 6 secondes (360 frames)  */
#define RESPAWN_DELAY_4  480   /* 8 secondes (480 frames)  */

/* ─── Structure d'un fantôme ─────────────────────────────────────────────── */
typedef struct {
    Vector2    pos;           /* Position sur la grille                    */
    Vector2    spawn;         /* Position de réapparition dans la maison   */
    Direction  dir;           /* Direction de déplacement courante         */
    GhostState state;         /* État courant                              */
    int        scared_timer;  /* Ticks restants en mode peur               */
    int        move_timer;    /* Pour gérer la vitesse + délai de sortie   */
    int        color_id;      /* Identifiant de couleur (0–3)              */
    int        exited;        /* 1 si le fantôme a quitté la maison        */
    int        dead_timer;    /* Ticks restants avant de ressortir        */
    int        respawn_delay; /* Délai de réapparition en ticks           */
    int        exit_delay;    /* Délai avant qu'il puisse sortir au départ */
} Ghost;

/* ─── Fonctions publiques ────────────────────────────────────────────────── */

/**
 * ghost_init  –  Initialise tous les fantômes à leurs positions de départ.
 * @param ghosts  Tableau de fantômes (taille GHOST_COUNT).
 */
void ghost_init(Ghost ghosts[GHOST_COUNT]);

/**
 * ghost_update  –  Met à jour la position d'un fantôme (IA + collisions).
 * @param ghost   Pointeur vers le fantôme.
 * @param map     Carte pour les collisions.
 * @param player_pos  Position du joueur (pour l'IA de poursuite simple).
 */
void ghost_update(Ghost *ghost, const Map *map, Vector2 player_pos);

/**
 * ghost_scare_all  –  Met tous les fantômes en état "peur" (super pastille).
 * @param ghosts  Tableau de fantômes.
 */
void ghost_scare_all(Ghost ghosts[GHOST_COUNT]);

/**
 * ghost_collides_with_player  –  Teste si un fantôme est sur la même case que le joueur.
 * @param ghost       Fantôme.
 * @param player_pos  Position du joueur.
 * @return            1 si collision, 0 sinon.
 */
int  ghost_collides_with_player(const Ghost *ghost, Vector2 player_pos);

/**
 * ghost_get_respawn_delay  –  Retourne le délai de réapparition pour un fantôme.
 * @param ghost_index  Index du fantôme (0-3).
 * @return             Délai en frames.
 */
int  ghost_get_respawn_delay(int ghost_index);

#endif /* GHOST_H */