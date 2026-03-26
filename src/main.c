/*
 * main.c
 * Point d'entrée du jeu Pacman en C avec SDL2.
 * Initialise SDL2, crée la fenêtre et lance la boucle de jeu.
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/game.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* Initialisation de SDL2 (vidéo uniquement pour le MVP) */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    /* Création de la fenêtre */
    SDL_Window *window = SDL_CreateWindow(
        "Pacman – Projet C SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Création du renderer (accélération matérielle + vsync) */
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    /* Initialisation et lancement du jeu */
    Game game;
    game_init(&game, renderer);
    game_run(&game, window, renderer);
    game_quit(&game);

    /* Nettoyage SDL2 */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
