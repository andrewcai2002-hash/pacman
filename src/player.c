/*
 * src/player.c
 * Implémentation du module joueur.
 */

#include "../include/player.h"

/* ─── Utilitaire : calcul de la case voisine ─────────────────────────────── */
static Vector2 next_pos(Vector2 pos, Direction dir)
{
    Vector2 np = pos;
    switch (dir) {
        case DIR_UP:    np.y -= 1; break;
        case DIR_DOWN:  np.y += 1; break;
        case DIR_LEFT:  np.x -= 1; break;
        case DIR_RIGHT: np.x += 1; break;
        default: break;
    }
    /* Tunnel horizontal : sortie côté gauche ↔ droite */
    if (np.x < 0)        np.x = MAP_COLS - 1;
    if (np.x >= MAP_COLS) np.x = 0;
    return np;
}

/* ─── Implémentation publique ────────────────────────────────────────────── */

void player_init(Player *player, int spawn_x, int spawn_y)
{
    player->pos.x     = spawn_x;
    player->pos.y     = spawn_y;
    player->spawn.x   = spawn_x;
    player->spawn.y   = spawn_y;
    player->dir       = DIR_NONE;
    player->next_dir  = DIR_NONE;
    player->lives     = PLAYER_START_LIVES;
    player->score     = 0;
    player->move_timer = 0;
    player->anim_frame = 0;
    player->anim_timer = 0;
}

void player_set_direction(Player *player, Direction dir)
{
    player->next_dir = dir;
}

int player_update(Player *player, Map *map)
{
    int collected = TILE_EMPTY;

    /* Animation de la bouche : alterne toutes les PLAYER_MOVE_DELAY/2 frames */
    player->anim_timer++;
    if (player->anim_timer >= PLAYER_MOVE_DELAY / 2) {
        player->anim_timer = 0;
        player->anim_frame = 1 - player->anim_frame;
    }

    /* Ralentissement : Pacman bouge 1 case toutes les PLAYER_MOVE_DELAY frames */
    player->move_timer++;
    if (player->move_timer < PLAYER_MOVE_DELAY)
        return collected;
    player->move_timer = 0;

    /* Essayer d'appliquer la direction mémorisée si possible */
    if (player->next_dir != DIR_NONE) {
        Vector2 try_pos = next_pos(player->pos, player->next_dir);
        if (map_is_walkable(map, try_pos.x, try_pos.y)) {
            player->dir = player->next_dir;
        }
    }

    /* Déplacement dans la direction courante */
    if (player->dir != DIR_NONE) {
        Vector2 np = next_pos(player->pos, player->dir);
        if (map_is_walkable(map, np.x, np.y)) {
            player->pos = np;
        }
    }

    /* Collecte de pastille sur la case actuelle */
    int tile = map_get_tile(map, player->pos.x, player->pos.y);
    if (tile == TILE_DOT || tile == TILE_SUPER) {
        collected = tile;
        map_set_tile(map, player->pos.x, player->pos.y, TILE_EMPTY);
        map->dot_count--;
        player->score += (tile == TILE_DOT) ? PLAYER_DOT_SCORE : PLAYER_SUPER_SCORE;
    }

    return collected;
}

void player_respawn(Player *player)
{
    player->pos      = player->spawn;
    player->dir      = DIR_NONE;
    player->next_dir = DIR_NONE;
    player->move_timer = 0;
    player->anim_frame = 0;
    player->anim_timer = 0;
    if (player->lives > 0)
        player->lives--;
}