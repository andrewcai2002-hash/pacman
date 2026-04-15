/*
 * src/ghost.c
 * Les fantômes et leur IA.
 */

#include <stdlib.h>
#include <time.h>
#include "../include/ghost.h"
#include "../include/types.h"

/* Sortie de la maison */
#define EXIT_X  13
#define EXIT_Y  12

/* Où ils commencent dans la maison */
static const int SPAWN_X[GHOST_COUNT] = {13, 13, 12, 14};
static const int SPAWN_Y[GHOST_COUNT] = {13, 14, 14, 14};

/* Délai de sortie de la maison au début du jeu (en frames) */
static const int EXIT_DELAY[GHOST_COUNT] = {0, 120, 300, 480};

/* Délai de réapparition après être mangé (en frames) */
static const int RESPAWN_DELAY[GHOST_COUNT] = {120, 240, 360, 480};

/* ─── Utilitaires ────────────────────────────────────────────────────────── */

/* Retourne la direction opposée */
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

/* Convertit une direction en vecteur */
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

/* Calcule la distance de Manhattan entre deux points */
static int manhattan(Vector2 a, Vector2 b)
{
    int dx = a.x - b.x; if (dx < 0) dx = -dx; /* Valeur absolue de dx */
    int dy = a.y - b.y; if (dy < 0) dy = -dy; /* Valeur absolue de dy */
    return dx + dy;
}

/* Vérifie si un fantôme peut marcher sur une case */
static int ghost_can_walk(const Map *map, int x, int y, int exited)
{
    int t = map_get_tile(map, x, y);
    if (t == TILE_WALL) return 0;                    /* Impossible : mur */
    if (t == TILE_GHOST_HOUSE && exited) return 0;  /* Impossible : maison (si sorti) */
    return 1; /* Possible */
}

/* ─── Implémentation publique ────────────────────────────────────────────── */

/* Les 4 directions */
static const Direction ALL_DIRS[4] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};

/* Initialise les fantômes */
void ghost_init(Ghost ghosts[GHOST_COUNT])
{
    int i;
    /* Initialiser le random seed pour l'IA (algo aléatoire) */
    srand((unsigned int)time(NULL));

    for (i = 0; i < GHOST_COUNT; i++) {
        ghosts[i].pos.x         = SPAWN_X[i];
        ghosts[i].pos.y         = SPAWN_Y[i];
        ghosts[i].spawn.x       = SPAWN_X[i];
        ghosts[i].spawn.y       = SPAWN_Y[i];
        ghosts[i].dir           = DIR_UP;           /* Direction initiale : vers le haut */
        ghosts[i].state         = GHOST_NORMAL;     /* État normal au démarrage */
        ghosts[i].scared_timer  = 0;                /* Pas de peur */
        ghosts[i].exited        = 0;                /* N'a pas encore quitté la maison */
        ghosts[i].dead_timer    = 0;                /* Pas mort */
        ghosts[i].respawn_delay = 0;                /* Pas en attente de réapparition */
        ghosts[i].exit_delay    = EXIT_DELAY[i];    /* Délai initial de sortie */
        ghosts[i].move_timer    = 0;                /* Aucun mouvement */
        ghosts[i].color_id      = i;                /* ID pour la couleur */
    }
}

/**
 * ghost_update  –  Met à jour la position et l'IA d'un fantôme
 * 
 * Phases d'un fantôme au cours du jeu:
 * 1. DEAD : mort (mangé) - attend dans la maison avant de réapparaître
 * 2. EXIT_DELAY : délai de sortie initial - attend avant de quitter la maison
 * 3. EXITING : sort de la maison en remontant vers la porte
 * 4. NORMAL ou SCARED : dehors - utilise l'IA de poursuite/fuite
 * 
 * L'IA est simple et basée sur la distance Manhattan :
 * - Mode NORMAL : choisir la direction qui rapproche le fantôme du joueur
 * - Mode SCARED : choisir la direction qui éloigne du joueur
 * 
 * Vitesse : les fantômes en mode SCARED sont légèrement plus lents
 * 
 * @param ghost      Pointeur vers le fantôme à mettre à jour
 * @param map        Carte pour les collisions
 * @param player_pos Position du joueur (pour l'IA de poursuite)
 */
void ghost_update(Ghost *ghost, const Map *map, Vector2 player_pos)
{
    int i;

    /* ── PHASE 1: État DEAD (mort) ── */
    /**
     * Si le fantôme est mort, décrémenter son timer de mort.
     * Quand il arrive à 0, il réapparaît à sa position de spawn
     * en mode NORMAL et doit ressortir de la maison.
     */
    if (ghost->state == GHOST_DEAD) {
        ghost->dead_timer--;
        if (ghost->dead_timer <= 0) {
            /* Temps d'attente écoulé → réapparition */
            ghost->state      = GHOST_NORMAL;      /* Retour en mode normal */
            ghost->exited     = 0;                 /* Marquer comme "à l'intérieur" */
            ghost->exit_delay = 0;                 /* Sort immédiatement après mort */
        }
        return; /* Ne rien faire d'autre si mort */
    }

    /* ── PHASE 2: Délai initial de sortie ── */
    /**
     * Au démarrage du jeu, les fantômes ont un délai avant de quitter la maison.
     * Cela décale les sorties (Blinky → Pinky → Inky → Clyde)
     */
    if (ghost->exit_delay > 0) {
        ghost->exit_delay--;
        return; /* Rester à l'intérieur et ne rien faire */
    }

    /* ── PHASE 3: Décrément du timer de peur ── */
    /**
     * Si en mode SCARED, décrémenter le timer.
     * Quand il arrive à 0, revenir en mode NORMAL.
     */
    if (ghost->state == GHOST_SCARED) {
        ghost->scared_timer--;
        if (ghost->scared_timer <= 0)
            ghost->state = GHOST_NORMAL; /* Fin du mode peur */
    }

    /* ── PHASE 4: Contrôle de la vitesse ── */
    /**
     * Les fantômes ne se déplacent pas tous les frames.
     * Ils bougent plus lentement en mode SCARED.
     */
    int speed = (ghost->state == GHOST_SCARED) ? GHOST_MOVE_DELAY + 6 : GHOST_MOVE_DELAY;
    ghost->move_timer++;
    if (ghost->move_timer < speed)
        return; /* Pas encore le moment de bouger */
    ghost->move_timer = 0;

    /* ── PHASE 5: Sortie de la maison ── */
    /**
     * Si le fantôme n'a pas encore quitté la maison (exited = 0),
     * il doit d'abord se diriger vers la porte de sortie.
     * 
     * Processus:
     * 1. Se centrer horizontalement à EXIT_X
     * 2. Monter verticalement vers EXIT_Y (la porte)
     * 3. Une fois à EXIT_X, EXIT_Y, marquer exited = 1
     */
    if (!ghost->exited) {
        /* ┌─ Se déplacer horizontalement vers le centre (EXIT_X) ─┐ */
        if (ghost->pos.x < EXIT_X && ghost_can_walk(map, ghost->pos.x + 1, ghost->pos.y, 0)) {
            ghost->pos.x++; /* Aller à droite */
        } else if (ghost->pos.x > EXIT_X && ghost_can_walk(map, ghost->pos.x - 1, ghost->pos.y, 0)) {
            ghost->pos.x--; /* Aller à gauche */
        } /* ┌─ Une fois centré, remonter vers la porte ─┐ */
        else if (ghost->pos.x == EXIT_X && ghost->pos.y > EXIT_Y
                   && ghost_can_walk(map, ghost->pos.x, ghost->pos.y - 1, 0)) {
            ghost->pos.y--; /* Monter */
        } /* ┌─ Vérifier si on est arrivé à la porte ─┐ */
        else if (ghost->pos.x == EXIT_X && ghost->pos.y == EXIT_Y) {
            ghost->exited = 1; /* Marquer comme "sorti" */
            ghost->dir = DIR_LEFT; /* Partir vers la gauche après la sortie */
        }
        return; /* Rester en phase de sortie, pas d'IA */
    }

    /* ── PHASE 6: IA de poursuite/fuite (quand dehors) ── */
    /**
     * Algorithme greedy simple:
     * - Tester les 4 directions (sauf celle d'où on vient)
     * - Pour chaque direction valide, calculer la distance Manhattan au joueur
     * - Choisir la direction qui minimise la distance (ou l'augmente en mode SCARED)
     */
    Direction best_dir = DIR_NONE;
    int best_score = -1;
    Vector2 best_pos;
    best_pos.x = ghost->pos.x;
    best_pos.y = ghost->pos.y;

    for (i = 0; i < 4; i++) {
        Direction d = ALL_DIRS[i];

        /* Ne pas revenir sur ses pas (sinon le fantôme oscille) */
        if (d == opposite(ghost->dir))
            continue;

        /* Calculer la position candidate */
        Vector2 dv = dir_to_vec(d);
        int nx = ghost->pos.x + dv.x;
        int ny = ghost->pos.y + dv.y;

        /* Tunnel horizontal */
        if (nx < 0)         nx = MAP_COLS - 1;
        if (nx >= MAP_COLS) nx = 0;

        /* Vérifier que la case suivante est marchable */
        if (!ghost_can_walk(map, nx, ny, ghost->exited))
            continue;

        Vector2 candidate = {nx, ny};
        int dist = manhattan(candidate, player_pos); /* Distance à la cible */

        /**
         * Ajouter une part d'aléatoire pour:
         * - Casser les patterns répétitifs
         * - Créer du unpredictability
         */
        if (rand() % 5 == 0)
            dist += (rand() % 5) - 2; /* Ajouter ±2 aléatoirement */

        /**
         * Choisir la meilleure direction selon le mode:
         * - SCARED : maximiser la distance (fuir)
         * - NORMAL : minimiser la distance (pourchasser)
         */
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

    /**
     * Fallback (demi-tour d'urgence) :
     * Si aucune direction n'a été trouvée (tous les côtés bloqués),
     * essayer de faire demi-tour.
     */
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

    /* Appliquer le mouvement décidé */
    if (best_dir != DIR_NONE) {
        ghost->dir = best_dir;
        ghost->pos = best_pos;
    }
}

/**
 * ghost_scare_all  –  Met tous les fantômes en mode "peur" (super pastille)
 * 
 * Quand le joueur collecte une super pastille, cette fonction:
 * - Change l'état de tous les fantômes (sauf les morts) à GHOST_SCARED
 * - Initialise le timer de peur à GHOST_SCARED_TICKS frames (~6 secondes)
 * 
 * @param ghosts  Tableau de fantômes (taille GHOST_COUNT)
 */
void ghost_scare_all(Ghost ghosts[GHOST_COUNT])
{
    int i;
    for (i = 0; i < GHOST_COUNT; i++) {
        /* Ne pas effrayer les fantômes morts */
        if (ghosts[i].state != GHOST_DEAD) {
            ghosts[i].state        = GHOST_SCARED;     /* Passer en mode peur */
            ghosts[i].scared_timer = GHOST_SCARED_TICKS; /* Durée en frames */
        }
    }
}

/**
 * ghost_collides_with_player  –  Teste si un fantôme est sur la même case que le joueur
 * 
 * Vérification simple: deux entités qui occupent la même case = collision.
 * Utilisée pour la détection du contact direct (pas le croisement).
 * 
 * @param ghost       Pointeur vers le fantôme
 * @param player_pos  Position du joueur
 * @return            1 si collision directe, 0 sinon
 */
int ghost_collides_with_player(const Ghost *ghost, Vector2 player_pos)
{
    return (ghost->pos.x == player_pos.x && ghost->pos.y == player_pos.y);
}

/**
 * ghost_get_respawn_delay  –  Retourne le délai de réapparition pour un fantôme
 * 
 * Quand un fantôme est mangé, il doit attendre un certain temps avant
 * de réapparaître. Plus l'indice du fantôme est grand, plus c'est long.
 * 
 * Délais:
 * - Fantôme 0 : 120 frames (~2 secondes)
 * - Fantôme 1 : 240 frames (~4 secondes)
 * - Fantôme 2 : 360 frames (~6 secondes)
 * - Fantôme 3 : 480 frames (~8 secondes)
 * 
 * @param ghost_index  Index du fantôme (0-3)
 * @return             Délai en frames
 */
int ghost_get_respawn_delay(int ghost_index)
{
    /* Vérifier que l'index est valide, sinon retourner le délai du fantôme 0 */
    if (ghost_index < 0 || ghost_index >= GHOST_COUNT)
        return RESPAWN_DELAY[0];
    return RESPAWN_DELAY[ghost_index];
}