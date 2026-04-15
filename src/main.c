/*
 * src/main.c
 * Point d'entrée du jeu Pacman.
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/game.h"

/*
 * main – Fonction principale
 * 
 * 1. Initialiser SDL2
 * 2. Créer la fenêtre
 * 3. Créer le renderer
 * 4. Initialiser le jeu
 * 5. Lancer la boucle
 * 6. Nettoyer
 */
int main(int argc, char *argv[])
{
    (void)argc;     /* Éviter le warning "unused parameter" */
    (void)argv;

    /* Initialiser SDL2 */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        return EXIT_FAILURE; /* Erreur : impossible de continuer */
    }

    /* Créer la fenêtre */
    SDL_Window *window = SDL_CreateWindow(
        "Pacman – Projet C SDL2",        /* Titre de la fenêtre */
        SDL_WINDOWPOS_UNDEFINED,          /* Position x (auto) */
        SDL_WINDOWPOS_UNDEFINED,          /* Position y (auto) */
        WINDOW_WIDTH,                     /* Largeur : 560 pixels */
        WINDOW_HEIGHT,                    /* Hauteur : 680 pixels */
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE /* Visible et redimensionnable */
    );
    
    /* Vérifier que la fenêtre a été créée */
    if (!window) {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        SDL_Quit(); /* Nettoyer SDL avant de quitter */
        return EXIT_FAILURE;
    }

    /* ────────────────────────────────────────────────────────────────── */
    /* Étape 3 : Création du renderer                                    */
    /* ────────────────────────────────────────────────────────────────── */
    /**
     * SDL_RENDERER_ACCELERATED : utiliser l'accélération matérielle (GPU)
     * SDL_RENDERER_PRESENTVSYNC : synchroniser avec l'écran (vsync)
     *                             limite automatiquement à ~60 FPS
     */
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    /* Vérifier que le renderer a été créé */
    if (!renderer) {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Initialiser et lancer le jeu */
    Game game;
    game_init(&game, renderer);     /* Initialiser la structure Game */
    game_run(&game, window, renderer); /* Lancer la boucle principale */
    game_quit(&game);               /* Libérer les ressources internes */

    /* ────────────────────────────────────────────────────────────────── */
    /* Étape 5 : Nettoyage SDL2                                          */
    /* ────────────────────────────────────────────────────────────────── */
    SDL_DestroyRenderer(renderer);  /* Libérer le renderer */
    SDL_DestroyWindow(window);      /* Libérer la fenêtre */
    SDL_Quit();                     /* Libérer toute SDL2 */

    return EXIT_SUCCESS; /* Quitter normalement */
}

