/*
 * src/player.c
 * Le joueur (Pacman).
 */

#include "../include/player.h"

/* Calcule la position suivante selon une direction avec wraparound horizontal */
static Vector2 next_pos(Vector2 pos, Direction dir)
{
    Vector2 np = pos; /* Copie de la position de départ */
    
    /* Appliquer le mouvement selon la direction */
    switch (dir) {
        case DIR_UP:    np.y -= 1; break;
        case DIR_DOWN:  np.y += 1; break;
        case DIR_LEFT:  np.x -= 1; break;
        case DIR_RIGHT: np.x += 1; break;
        default: break;
    }
    
    /**
     * Tunnel horizontal :
     * Bouclage sur l'axe horizontal pour créer un tunnel (style Pac-Man original)
     */
    if (np.x < 0)        np.x = MAP_COLS - 1; /* Sortie gauche → droite */
    if (np.x >= MAP_COLS) np.x = 0;           /* Sortie droite → gauche */
    
    return np;
}

/* ─── Implémentation publique ────────────────────────────────────────────── */

/* Initialise le joueur au départ */
void player_init(Player *player, int spawn_x, int spawn_y)
{
    player->pos.x     = spawn_x;
    player->pos.y     = spawn_y;
    player->spawn.x   = spawn_x;
    player->spawn.y   = spawn_y;
    player->dir       = DIR_NONE;              /* Pas de mouvement initial */
    player->next_dir  = DIR_NONE;              /* Pas de direction en attente */
    player->lives     = PLAYER_START_LIVES;    /* 3 vies au départ */
    player->score     = 0;                     /* Score initial 0 */
    player->move_timer = 0;                    /* Aucun déplacement */
    player->anim_frame = 0;                    /* Bouche ouverte */
    player->anim_timer = 0;                    /* Animation à zéro */
}

/* Mémorise la direction qu'on veut prendre */
void player_set_direction(Player *player, Direction dir)
{
    player->next_dir = dir; /* Mémoriser la prochaine direction souhaitée */
}

/* Bouge le joueur et collecte les pastilles */
int player_update(Player *player, Map *map)
{
    int collected = TILE_EMPTY; /* Par défaut aucune pastille */

    /**
     * Animation de la bouche :
     * - Alterne entre 0 (ouvert) et 1 (fermé) 
     * - Changeé toutes les PLAYER_MOVE_DELAY/2 frames
     */
    player->anim_timer++;
    if (player->anim_timer >= PLAYER_MOVE_DELAY / 2) {
        player->anim_timer = 0;
        player->anim_frame = 1 - player->anim_frame; /* Basculer 0 <-> 1 */
    }

    /**
     * Ralentissement : Pac-Man se déplace 1 case tous les PLAYER_MOVE_DELAY frames
     * (par défaut 8 frames = environ 7 cases/seconde à 60 FPS)
     */
    player->move_timer++;
    if (player->move_timer < PLAYER_MOVE_DELAY)
        return collected; /* Pas encore le moment de se déplacer */
    player->move_timer = 0;

    /**
     * Virage anticipé :
     * Si next_dir est défini et qu'on peut se déplacer dans cette direction,
     * l'appliquer immédiatement. Cela permet au joueur de préparer un virage
     * avant d'arriver à l'intersection.
     */
    if (player->next_dir != DIR_NONE) {
        Vector2 try_pos = next_pos(player->pos, player->next_dir);
        if (map_is_walkable(map, try_pos.x, try_pos.y)) {
            player->dir = player->next_dir; /* Accepter le nouveau virage */
        }
    }

    /**
     * Déplacement dans la direction courante
     * (en général la direction qu'on a maintenue, ou le virage anticipé)
     */
    if (player->dir != DIR_NONE) {
        Vector2 np = next_pos(player->pos, player->dir);
        /* Vérifier que la case suivante est franchissable (pas un mur) */
        if (map_is_walkable(map, np.x, np.y)) {
            player->pos = np; /* Appliquer le déplacement */
        }
    }

    /**
     * Collecte de pastille sur la case courante
     * Si la case contient une pastille, l'enlever de la carte et ajouter des points
     */
    int tile = map_get_tile(map, player->pos.x, player->pos.y);
    if (tile == TILE_DOT || tile == TILE_SUPER) {
        collected = tile;
        map_set_tile(map, player->pos.x, player->pos.y, TILE_EMPTY); /* Enlever la pastille */
        map->dot_count--; /* Décrémenter le compteur */
        
        /* Ajouter les points correspondants */
        player->score += (tile == TILE_DOT) ? PLAYER_DOT_SCORE : PLAYER_SUPER_SCORE;
    }

    return collected;
}

/**
 * player_respawn  –  Ramène le joueur à sa position de spawn et perd une vie
 * 
 * Appelée quand le joueur entre en collision avec un fantôme normal.
 * Réinitialise:
 * - Position (revenir au spawn)
 * - Direction et mouvement (s'arrêter)
 * - Animation (remettre à zéro)
 * - Vies (perdre 1 vie)
 * 
 * @param player  Pointeur vers le joueur
 */
void player_respawn(Player *player)
{
    player->pos      = player->spawn;  /* Retourner au spawn */
    player->dir      = DIR_NONE;       /* S'arrêter */
    player->next_dir = DIR_NONE;       /* Aucune direction en attente */
    player->move_timer = 0;            /* Réinitialiser le timer de mouvement */
    player->anim_frame = 0;            /* Bouche ouverte */
    player->anim_timer = 0;            /* Animation à zéro */
    
    /* Perdre une vie */
    if (player->lives > 0)
        player->lives--;
}