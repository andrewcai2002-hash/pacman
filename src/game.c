/*
 * src/game.c
 * Implémentation du module principal du jeu.
 *
 * Flux de la boucle :
 *   1. Gestion des événements SDL (clavier, fermeture)
 *   2. Mise à jour logique (joueur, fantômes, collisions, score)
 *   3. Rendu visuel
 *   4. Attente pour respecter le framerate cible
 *
 * Utilise toujours la carte DEFAULT_MAP depuis map.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/game.h"
#include "../include/render.h"

/* ─── Constantes internes ────────────────────────────────────────────────── */
#define PLAYER_SPAWN_X  13
#define PLAYER_SPAWN_Y  23
#define MAX_LEVELS       2

/* ─── Chargement / sauvegarde du high score ──────────────────────────────── */
static int load_high_score(void)
{
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    int hs = 0;
    if (f) {
        fscanf(f, "%d", &hs);
        fclose(f);
    }
    return hs;
}

static void save_high_score(int score)
{
    FILE *f = fopen(HIGHSCORE_FILE, "w");
    if (f) {
        fprintf(f, "%d\n", score);
        fclose(f);
    }
}

/* ─── Chargement de la carte du niveau courant ───────────────────────────── */
static void load_level_map(Game *game)
{
    /* Utilise toujours la carte DEFAULT_MAP depuis map.c */
    (void)game;
    map_load(&game->map);
}

/* ─── Implémentation publique ────────────────────────────────────────────── */

void game_init(Game *game, SDL_Renderer *renderer)
{
    (void)renderer;
    game->state      = STATE_MENU;
    game->high_score = load_high_score();
    game->tick       = 0;
    game->level      = 1;
    game_reset(game);
}

void game_reset(Game *game)
{
    load_level_map(game);
    player_init(&game->player, PLAYER_SPAWN_X, PLAYER_SPAWN_Y);
    ghost_init(game->ghosts);
    game->tick = 0;
    game->ghosts_eaten_count = 0;  /* Réinitialiser le compteur de fantômes mangés */
}

/* ─── Gestion des événements ─────────────────────────────────────────────── */
static void handle_events(Game *game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->state = STATE_QUIT;
            return;
        }

        if (event.type == SDL_KEYDOWN) {
            int key = event.key.keysym.sym;

            /* ── Menu principal ── */
            if (game->state == STATE_MENU) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    game->level = 1;
                    game_reset(game);
                    game->state = STATE_PLAYING;
                }
                if (key == SDLK_ESCAPE)
                    game->state = STATE_QUIT;
                return;
            }

            /* ── Victoire : niveau suivant ou fin du jeu ── */
            if (game->state == STATE_WIN) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    if (game->level < MAX_LEVELS) {
                        int saved_score = game->player.score;
                        int saved_lives = game->player.lives;
                        game->level++;
                        game_reset(game);
                        game->player.score = saved_score;
                        game->player.lives = saved_lives;
                        game->state = STATE_PLAYING;
                    } else {
                        /* Tous les niveaux terminés → retour menu */
                        game->level = 1;
                        game_reset(game);
                        game->state = STATE_MENU;
                    }
                }
                if (key == SDLK_ESCAPE)
                    game->state = STATE_QUIT;
                return;
            }

            /* ── Game over : recommencer depuis le niveau 1 ── */
            if (game->state == STATE_GAME_OVER) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    game->level = 1;
                    game_reset(game);
                    game->state = STATE_PLAYING;
                }
                if (key == SDLK_ESCAPE)
                    game->state = STATE_QUIT;
                return;
            }

            /* ── Contrôles en jeu ── */
            if (game->state == STATE_PLAYING) {
                switch (key) {
                    case SDLK_UP:    case SDLK_z: case SDLK_w:
                        player_set_direction(&game->player, DIR_UP);    break;
                    case SDLK_DOWN:  case SDLK_s:
                        player_set_direction(&game->player, DIR_DOWN);  break;
                    case SDLK_LEFT:  case SDLK_q: case SDLK_a:
                        player_set_direction(&game->player, DIR_LEFT);  break;
                    case SDLK_RIGHT: case SDLK_d:
                        player_set_direction(&game->player, DIR_RIGHT); break;
                    case SDLK_ESCAPE:
                        game->state = STATE_MENU;                       break;
                    default: break;
                }
            }
        }
    }
}

/* ─── Mise à jour logique ────────────────────────────────────────────────── */
static void update(Game *game)
{
    if (game->state != STATE_PLAYING)
        return;

    game->tick++;

    /* ── Joueur ── */
    int collected = player_update(&game->player, &game->map);

    if (collected == TILE_SUPER)
        ghost_scare_all(game->ghosts);

    if (game->player.score > game->high_score)
        game->high_score = game->player.score;

    /* ── Fantômes ── */
    int i;
    int any_scared = 0;  /* Vérifier s'il reste des fantômes en mode bleu */
    
    for (i = 0; i < GHOST_COUNT; i++) {
        ghost_update(&game->ghosts[i], &game->map, game->player.pos);
        
        /* Vérifier s'il reste des fantômes effrayés */
        if (game->ghosts[i].state == GHOST_SCARED)
            any_scared = 1;

        if (ghost_collides_with_player(&game->ghosts[i], game->player.pos)) {
            if (game->ghosts[i].state == GHOST_SCARED) {
                /* Points exponentiels : 200, 400, 800, 1600 */
                int points = 0;
                switch (game->ghosts_eaten_count) {
                    case 0: points = 200;   break;
                    case 1: points = 400;   break;
                    case 2: points = 800;   break;
                    case 3: points = 1600;  break;
                    default: points = 200;  break;
                }
                game->player.score += points;
                game->ghosts_eaten_count++;
                
                /* Envoyer le fantôme à la maison pour réapparition */
                game->ghosts[i].pos   = game->ghosts[i].spawn;
                game->ghosts[i].state = GHOST_DEAD;
                game->ghosts[i].exited = 0;
                game->ghosts[i].dead_timer = ghost_get_respawn_delay(i);
            } else if (game->ghosts[i].state == GHOST_NORMAL) {
                player_respawn(&game->player);
                ghost_init(game->ghosts);
                game->ghosts_eaten_count = 0;  /* Réinitialiser après mort */
                if (game->player.lives <= 0)
                    game->state = STATE_GAME_OVER;
                return;
            }
        }
    }
    
    /* Réinitialiser le compteur si aucun fantôme n'est plus effrayé */
    if (!any_scared)
        game->ghosts_eaten_count = 0;

    /* ── Condition de victoire ── */
    if (game->map.dot_count <= 0)
        game->state = STATE_WIN;
}

/* ─── Rendu complet d'une frame ──────────────────────────────────────────── */
static void render_frame(Game *game, SDL_Renderer *renderer)
{
    render_clear(renderer);

    switch (game->state) {
        case STATE_MENU:
            render_map(renderer, &game->map, game->tick);
            render_overlay_text(renderer, "PRESS ENTER");
            break;

        case STATE_PLAYING:
            render_map(renderer, &game->map, game->tick);
            render_player(renderer, &game->player);
            render_ghosts(renderer, game->ghosts);
            render_hud(renderer, &game->player, game->level, game->high_score);
            break;

        case STATE_WIN:
            render_map(renderer, &game->map, game->tick);
            render_hud(renderer, &game->player, game->level, game->high_score);
            if (game->level < MAX_LEVELS)
                render_overlay_text(renderer, "NEXT LEVEL!");
            else
                render_overlay_text(renderer, "YOU WIN!");
            break;

        case STATE_GAME_OVER:
            render_map(renderer, &game->map, game->tick);
            render_hud(renderer, &game->player, game->level, game->high_score);
            render_overlay_text(renderer, "GAME OVER");
            break;

        default:
            break;
    }

    render_present(renderer);
}

/* ─── Boucle principale ──────────────────────────────────────────────────── */
void game_run(Game *game, SDL_Window *window, SDL_Renderer *renderer)
{
    (void)window;
    Uint32 frame_start;
    Uint32 elapsed;

    while (game->state != STATE_QUIT) {
        frame_start = SDL_GetTicks();

        handle_events(game);
        update(game);
        render_frame(game, renderer);

        elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < FRAME_DELAY_MS)
            SDL_Delay(FRAME_DELAY_MS - elapsed);
    }
}

void game_quit(Game *game)
{
    save_high_score(game->high_score);
}