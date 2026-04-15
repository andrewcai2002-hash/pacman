/*
 * src/game.c
 * Le moteur du jeu : états, menus, boucle principale.
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/game.h"
#include "../include/render.h"

/* ─── Constantes internes ────────────────────────────────────────────────── */
/* Le joueur spawn au milieu de la map */
#define PLAYER_SPAWN_X  13
#define PLAYER_SPAWN_Y  23
#define MAX_LEVELS       2

/* Charge/sauvegarde du high score */

/* Charge le high score depuis le fichier */
static int load_high_score(void)
{
    /* Tentative d'ouverture du fichier en lecture */
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    int hs = 0; /* Valeur par défaut si lecture échoue */
    
    /* Si le fichier existe */
    if (f) {
        /* Lire le premier entier du fichier */
        fscanf(f, "%d", &hs);
        fclose(f); /* Fermer le fichier après lecture */
    }
    return hs;
}

/* Sauvegarde le high score */
static void save_high_score(int score)
{
    /* Ouvrir le fichier en mode écriture (crée ou écrase) */
    FILE *f = fopen(HIGHSCORE_FILE, "w");
    if (f) {
        /* Écrire le score avec un saut de ligne */
        fprintf(f, "%d\n", score);
        fclose(f); /* Fermer le fichier après écriture */
    }
}


/* Charge la carte du niveau */
static void load_level_map(Game *game)
{
    /* Appeler map_load pour initialiser la carte */
    (void)game; /* Cast pour éviter le warning "unused parameter" */
    map_load(&game->map);
}

/* Définitions publiques */

/* Initialise le jeu */
void game_init(Game *game, SDL_Renderer *renderer)
{
    (void)renderer; /* Cast pour éviter warning "unused parameter" */
    game->state      = STATE_MENU;              /* Commencer sur l'écran de menu */
    game->high_score = load_high_score();       /* Charger le meilleur score */
    game->tick       = 0;                       /* Initialiser le compteur de ticks */
    game->level      = 1;                       /* Commencer au niveau 1 */
    game_reset(game);                           /* Réinitialiser map, joueur, fantômes */
}

/* Réinitialise la carte, le joueur et les fantômes */
void game_reset(Game *game)
{
    load_level_map(game);               /* Charger/réinitialiser la carte */
    player_init(&game->player, PLAYER_SPAWN_X, PLAYER_SPAWN_Y); /* Joueur au spawn */
    ghost_init(game->ghosts);           /* Tous les fantômes aux spawns */
    game->tick = 0;                     /* Réinitialiser le compteur */
    game->ghosts_eaten_count = 0;       /* Aucun fantôme mangé au début */
}

/* ─── Gestion des événements ─────────────────────────────────────────────── */

/* Gère les entrées clavier et la fermeture */
static void handle_events(Game *game)
{
    SDL_Event event;
    
    /* Boucler jusqu'à ce que tous les événements soient traités */
    while (SDL_PollEvent(&event)) {
        /* Fermeture de la fenêtre */
        if (event.type == SDL_QUIT) {
            game->state = STATE_QUIT;
            return;
        }

        /* Traiter uniquement les appuis de touches (ignorer les relâchements) */
        if (event.type == SDL_KEYDOWN) {
            int key = event.key.keysym.sym;

            /* ─── État MENU ─── */
            if (game->state == STATE_MENU) {
                /* Entrée pour démarrer la partie */
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    game->level = 1;
                    game_reset(game);
                    game->state = STATE_PLAYING;
                }
                /* Échap pour quitter le jeu */
                if (key == SDLK_ESCAPE)
                    game->state = STATE_QUIT;
                return;
            }

            /* ─── État WIN (victoire) ─── */
            if (game->state == STATE_WIN) {
                /* Entrée pour continuer */
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    if (game->level < MAX_LEVELS) {
                        /* Niveau suivant : sauvegarder score et vies */
                        int saved_score = game->player.score;
                        int saved_lives = game->player.lives;
                        game->level++;
                        game_reset(game);
                        /* Restaurer le score et les vies pour le nouveau niveau */
                        game->player.score = saved_score;
                        game->player.lives = saved_lives;
                        game->state = STATE_PLAYING;
                    } else {
                        /* Tous les niveaux terminés : retour au menu */
                        game->level = 1;
                        game_reset(game);
                        game->state = STATE_MENU;
                    }
                }
                /* Échap pour retourner au menu */
                if (key == SDLK_ESCAPE)
                    game->state = STATE_QUIT;
                return;
            }

            /* ─── État GAME_OVER ─── */
            if (game->state == STATE_GAME_OVER) {
                /* Entrée pour rejoner */
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    game->level = 1;
                    game_reset(game);
                    game->state = STATE_PLAYING;
                }
                /* Échap pour quitter */
                if (key == SDLK_ESCAPE)
                    game->state = STATE_QUIT;
                return;
            }

            /* ─── État PLAYING (partie en cours) ─── */
            if (game->state == STATE_PLAYING) {
                /* Traiter les touches de direction (4 touches par direction) */
                switch (key) {
                    /* Haut : Flèche Haut, Z (AZERTY), W (QWERTY) */
                    case SDLK_UP:    case SDLK_z: case SDLK_w:
                        player_set_direction(&game->player, DIR_UP);    break;
                    /* Bas : Flèche Bas, S (AZERTY/QWERTY) */
                    case SDLK_DOWN:  case SDLK_s:
                        player_set_direction(&game->player, DIR_DOWN);  break;
                    /* Gauche : Flèche Gauche, Q (AZERTY), A (QWERTY) */
                    case SDLK_LEFT:  case SDLK_q: case SDLK_a:
                        player_set_direction(&game->player, DIR_LEFT);  break;
                    /* Droite : Flèche Droite, D (AZERTY/QWERTY) */
                    case SDLK_RIGHT: case SDLK_d:
                        player_set_direction(&game->player, DIR_RIGHT); break;
                    /* Échap pour retourner au menu pendant le jeu */
                    case SDLK_ESCAPE:
                        game->state = STATE_MENU;                       break;
                    default: break;
                }
            }
        }
    }
}

/* ─── Mise à jour logique ────────────────────────────────────────────────── */

/**
 * update  –  Met à jour la logique du jeu (joueur, fantômes, collisions, score)
 * 
 * Cette fonction gère:
 * 1. Le déplacement du joueur et la collecte de pastilles
 * 2. Les appels de mise à jour des fantômes
 * 3. La détection de collisions (joueur vs fantômes)
 * 4. Les points gagnés (mangé des fantômes, nouvelles pastilles)
 * 5. La détection de victoire (toutes les pastilles collectées)
 * 
 * Logique des collisions:
 * - Direct : joueur et fantôme sur la même case
 * - Swap : le joueur et le fantôme se croisent en échangeant leurs positions
 * 
 * Gestion du mode "peur" (super pastille):
 * - Les fantômes changent en GHOST_SCARED pendant GHOST_SCARED_TICKS frames
 * - Les points sont exponentiels : 200, 400, 800, 1600 pour chaque fantôme mangé
 * - Le compteur de fantômes mangés se réinitialise quand aucun n'est plus effrayé
 * 
 * @param game  Pointeur vers la structure Game
 */
static void update(Game *game)
{
    /* Ne mettre à jour que si le jeu est en cours */
    if (game->state != STATE_PLAYING)
        return;

    game->tick++;

    /* Sauvegarder la position du joueur avant déplacement (pour détection de swap) */
    Vector2 player_pos_before = game->player.pos;

    /* ── Mise à jour du joueur ── */
    /**
     * player_update retourne:
     * - TILE_DOT si une pastille standard a été collectée
     * - TILE_SUPER si une super pastille a été collectée
     * - TILE_EMPTY sinon
     */
    int collected = player_update(&game->player, &game->map);

    /* Si super pastille collectée, mettre tous les fantômes en mode peur */
    if (collected == TILE_SUPER)
        ghost_scare_all(game->ghosts);

    /* Mettre à jour le high score si le score courant le dépasse */
    if (game->player.score > game->high_score)
        game->high_score = game->player.score;

    /* ── Mise à jour des fantômes ── */
    int i;
    int any_scared = 0; /* Drapeau : y a-t-il au moins un fantôme en mode peur */

    for (i = 0; i < GHOST_COUNT; i++) {
        Vector2 ghost_pos_before = game->ghosts[i].pos;

        /* Mettre à jour la position et l'IA du fantôme */
        ghost_update(&game->ghosts[i], &game->map, game->player.pos);

        /* Vérifier s'il y a au moins un fantôme en mode peur */
        if (game->ghosts[i].state == GHOST_SCARED)
            any_scared = 1;

        /* ── Détection de collision ── */
        /**
         * On teste deux types de collision:
         * 1. Direct : joueur et fantôme sur la même case actuellement
         * 2. Swap : le joueur était sur la case du fantôme et le fantôme
         *    était sur la case du joueur (ils se sont croisés)
         */
        int direct_collision = ghost_collides_with_player(
            &game->ghosts[i], game->player.pos);

        int swap_collision =
            (game->ghosts[i].pos.x == player_pos_before.x &&
             game->ghosts[i].pos.y == player_pos_before.y &&
             ghost_pos_before.x    == game->player.pos.x  &&
             ghost_pos_before.y    == game->player.pos.y);

        /* Gérer la collision selon l'état du fantôme */
        if (direct_collision || swap_collision) {
            if (game->ghosts[i].state == GHOST_SCARED) {
                /* ─── Fantôme effrayé mangé ─── */
                /**
                 * Points exponentiels pour manger des fantômes en cascade :
                 * 1er fantôme mangé : 200 points
                 * 2eme fantôme mangé : 400 points
                 * 3eme fantôme mangé : 800 points
                 * 4eme fantôme mangé : 1600 points
                 */
                int points = 200;
                switch (game->ghosts_eaten_count) {
                    case 0: points = 200;  break;
                    case 1: points = 400;  break;
                    case 2: points = 800;  break;
                    case 3: points = 1600; break;
                    default: points = 200; break;
                }
                game->player.score += points;
                game->ghosts_eaten_count++;

                /* Réinitialiser le fantôme : il retourne se cacher à la maison */
                game->ghosts[i].pos    = game->ghosts[i].spawn;
                game->ghosts[i].state  = GHOST_DEAD;
                game->ghosts[i].exited = 0;
                game->ghosts[i].dead_timer = ghost_get_respawn_delay(i);

            } else if (game->ghosts[i].state == GHOST_NORMAL) {
                /* ─── Fantôme normal : joueur perd une vie ─── */
                player_respawn(&game->player);      /* Ramener le joueur au spawn */
                ghost_init(game->ghosts);           /* Réinitialiser tous les fantômes */
                game->ghosts_eaten_count = 0;       /* Réinitialiser le compteur */
                
                /* Vérifier si c'est game over */
                if (game->player.lives <= 0)
                    game->state = STATE_GAME_OVER;
                return;
            }
        }
    }

    /* ── Réinitialiser le compteur de fantômes mangés ── */
    /**
     * Si aucun fantôme n'est plus en mode peur, réinitialiser le compteur
     * pour que le prochain power-up recommence à 200 points
     */
    if (!any_scared)
        game->ghosts_eaten_count = 0;

    /* ── Condition de victoire ── */
    /**
     * Si dot_count <= 0, le joueur a collecté toutes les pastilles.
     * Passer à l'état WIN pour afficher l'écran de victoire.
     */
    if (game->map.dot_count <= 0)
        game->state = STATE_WIN;
}

/* ─── Rendu complet d'une frame ──────────────────────────────────────────── */

/**
 * render_frame  –  Affiche une frame complète selon l'état du jeu
 * 
 * Cette fonction interne gère l'affichage global selon l'état du jeu:
 * - STATE_MENU: Affiche la carte avec overlay "SCORE"
 * - STATE_PLAYING: Affiche map, joueur, fantômes, HUD
 * - STATE_WIN: Affiche map, HUD, et overlay "SCORE" ou "WIN"
 * - STATE_GAME_OVER: Affiche map, HUD, et overlay "OVER"
 * 
 * @param game      Pointeur vers la structure Game
 * @param renderer  Renderer SDL2
 */
static void render_frame(Game *game, SDL_Renderer *renderer)
{
    /* Effacer l'écran avec le fond noir */
    render_clear(renderer);

    /* Afficher les éléments selon l'état du jeu */
    switch (game->state) {
        case STATE_MENU:
            /* Menu principal : afficher la carte et un message */
            render_map(renderer, &game->map, game->tick);
            render_overlay_text(renderer, "SCORE");   /* "PRESS ENTER" trop long sans TTF */
            break;

        case STATE_PLAYING:
            /* Jeu en cours : afficher tous les éléments */
            render_map(renderer, &game->map, game->tick);
            render_player(renderer, &game->player);
            render_ghosts(renderer, game->ghosts);
            render_hud(renderer, &game->player, game->level, game->high_score);
            break;

        case STATE_WIN:
            /* Écran de victoire */
            render_map(renderer, &game->map, game->tick);
            render_hud(renderer, &game->player, game->level, game->high_score);
            /* Afficher "SCORE" si plus de niveaux, "WIN" si c'était le dernier */
            if (game->level < MAX_LEVELS)
                render_overlay_text(renderer, "SCORE");   /* placeholder */
            else
                render_overlay_text(renderer, "WIN");
            break;

        case STATE_GAME_OVER:
            /* Écran de game over */
            render_map(renderer, &game->map, game->tick);
            render_hud(renderer, &game->player, game->level, game->high_score);
            render_overlay_text(renderer, "OVER");
            break;

        default:
            break;
    }

    /* Envoyer le rendu à l'écran (swap front/back buffer) */
    render_present(renderer);
}

/* ─── Boucle principale ──────────────────────────────────────────────────── */

/**
 * game_run  –  Lance et maintient la boucle de jeu
 * 
 * Cette est la boucle principale du jeu. Elle:
 * 1. Traite les événements clavier/souris
 * 2. Met à jour la logique du jeu (joueur, fantômes, collisions)
 * 3. Affiche une nouvelle frame
 * 4. Contrôle la fréquence d'images (FPS) à 60
 * 
 * Elle tourne en continu jusqu'à ce que game->state == STATE_QUIT.
 * 
 * @param game      Pointeur vers la structure Game
 * @param window    Fenêtre SDL2 (paramètre conservé pour évolutions futures)
 * @param renderer  Renderer SDL2
 */
void game_run(Game *game, SDL_Window *window, SDL_Renderer *renderer)
{
    (void)window; /* Cast pour éviter warning "unused parameter" */
    Uint32 frame_start;
    Uint32 elapsed;

    /**
     * Boucle de jeu : continue jusqu'à ce que l'état passe à STATE_QUIT
     */
    while (game->state != STATE_QUIT) {
        /* Enregistrer le temps de départ de la frame */
        frame_start = SDL_GetTicks();

        /* 1. Traiter les événements (clavier, fermeture, etc.) */
        handle_events(game);
        
        /* 2. Mettre à jour la logique */
        update(game);
        
        /* 3. Afficher la frame */
        render_frame(game, renderer);

        /**
         * 4. Limite de framerate : 
         *    - Calculer le temps écoulé pour cette frame
         *    - Si inférieur à FRAME_DELAY_MS, attendre le reste
         *    - Cela limite à TARGET_FPS = 60 FPS
         */
        elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < FRAME_DELAY_MS)
            SDL_Delay(FRAME_DELAY_MS - elapsed);
    }
}

/**
 * game_quit  –  Libère les ressources et sauvegarde les données persistantes
 * 
 * Appelée à la fermeture du jeu pour:
 * - Sauvegarder le meilleur score dans le fichier
 * 
 * @param game  Pointeur vers la structure Game
 */
void game_quit(Game *game)
{
    save_high_score(game->high_score);
}