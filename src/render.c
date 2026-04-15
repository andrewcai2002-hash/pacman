/*
 * src/render.c
 * Module rendu et affichage avec SDL2.
 */

#include <string.h>
#include "../include/render.h"

/* ─── Utilitaire : dessiner un cercle rempli ─────────────────────────────── */

/* Dessine un cercle rempli par approximation (teste chaque pixel d'une boîte englobante) */
/*
 * Utilisée pour afficher le joueur et les fantômes.
 * On teste si x² + y² ≤ radius² pour chaque pixel.
 */
static void fill_circle(SDL_Renderer *r, int cx, int cy, int radius)
{
    int x, y;
    for (y = -radius; y <= radius; y++) {
        for (x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius)
                SDL_RenderDrawPoint(r, cx + x, cy + y);

        }
    }
}

/* ─── Conversion position grille → pixel (centre de la case) ────────────── */

/* Convertit une colonne grille en coordonnée x écran (center) */
/* Formule : col * TILE_SIZE + TILE_SIZE/2 */
static int tile_to_px(int col) { return col * TILE_SIZE + TILE_SIZE / 2; }

/* Convertit une ligne grille en coordonnée y écran (center) */
/* Formule : row * TILE_SIZE + TILE_SIZE/2 */
static int tile_to_py(int row) { return row * TILE_SIZE + TILE_SIZE / 2; }

/* ─── Rendu d'un caractère en bitmap 5×7 ────────────────────────────────── */
/*
 * Chaque lettre : 5 colonnes × 7 lignes = 35 bits encodés en long long.
 * Bit 34 = pixel (0,0), bit 0 = pixel (4,6).
 */
static void draw_char_bitmap(SDL_Renderer *renderer, int ox, int oy, char c)
{
    /* Bitmaps 5×7 pour A-Z, 0-9, espace */
    /* Chaque entrée = 35 bits, stockés dans un unsigned long long */
    static const unsigned long long BITMAPS[37] = {
        /* A */ 0x74A3E8A00ULL >> 4,
        /* B */ 0x69E9A6E00ULL >> 4,
        /* C */ 0x72108C600ULL >> 4,
        /* D */ 0x69108A600ULL >> 4, /* simplifié */
        /* E */ 0x78E08C700ULL >> 4,
        /* F */ 0x78E08C000ULL >> 4,
        /* G */ 0x72138A600ULL >> 4,
        /* H */ 0x528FA8A00ULL >> 4,
        /* I */ 0x74210C700ULL >> 4,
        /* J */ 0x1084A6400ULL >> 4,
        /* K */ 0x528C28A00ULL >> 4,
        /* L */ 0x42108C700ULL >> 4,
        /* M */ 0x57508A800ULL >> 4,
        /* N */ 0x59748A800ULL >> 4,
        /* O */ 0x72308A600ULL >> 4,
        /* P */ 0x74A3C8000ULL >> 4,
        /* Q */ 0x72308A610ULL >> 4,
        /* R */ 0x74A3C8A00ULL >> 4,
        /* S */ 0x72302C600ULL >> 4,
        /* T */ 0x74210C200ULL >> 4,
        /* U */ 0x528A8A600ULL >> 4,
        /* V */ 0x528A8C400ULL >> 4,
        /* W */ 0x52BAEA400ULL >> 4,
        /* X */ 0x528C28A00ULL >> 4,
        /* Y */ 0x528C20800ULL >> 4,
        /* Z */ 0x7840C8700ULL >> 4,
        /* 0 */ 0x72B4CA600ULL >> 4,
        /* 1 */ 0x21084210ULL,        /* simple : colonne centrale */
        /* 2 */ 0x72042C700ULL >> 4,
        /* 3 */ 0x720C0A600ULL >> 4,
        /* 4 */ 0x528FA0800ULL >> 4,
        /* 5 */ 0x78E02C600ULL >> 4,
        /* 6 */ 0x72E8AC600ULL >> 4,
        /* 7 */ 0x72108A000ULL >> 4, /* simplifié */
        /* 8 */ 0x72FA8A600ULL >> 4,
        /* 9 */ 0x72FA0C600ULL >> 4,
        /* ' ' */ 0x0ULL
    };

    /*
     * Bitmap simplifié : on dessine directement des rectangles
     * pour chaque lettre connue. Plus fiable que le bitmap ci-dessus.
     */

    int ps = 2; /* Taille d'un pixel du bitmap */
    SDL_Rect r;

    /* On dessine selon le caractère */

    switch(c) {
        case 'S':
            r = (SDL_Rect){ox,      oy,      5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+ps,   ps,   2*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+3*ps, 5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy+4*ps, ps,   2*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+6*ps, 5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'C':
            r = (SDL_Rect){ox,      oy,      5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+ps,   ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+6*ps, 5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'O':
            r = (SDL_Rect){ox+ps,   oy,      3*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+ps,   ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy+ps,   ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+6*ps, 3*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'R':
            r = (SDL_Rect){ox,      oy,      4*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+ps,   ps,   6*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy+ps,   ps,   2*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+3*ps, 4*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+4*ps, ps,   ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+3*ps, oy+5*ps, ps,   ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy+6*ps, ps,   ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'E':
            r = (SDL_Rect){ox, oy,      5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox, oy+ps,   ps,   2*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox, oy+3*ps, 4*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox, oy+4*ps, ps,   2*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox, oy+6*ps, 5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'H':
            r = (SDL_Rect){ox,      oy, ps, 7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy, ps, 7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+3*ps, 2*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'I':
            r = (SDL_Rect){ox,      oy,      5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+ps,   ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+6*ps, 5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'G':
            r = (SDL_Rect){ox+ps,   oy,      4*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+ps,   ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+6*ps, 4*ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy+3*ps, ps,   3*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+3*ps, 2*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'L':
            r = (SDL_Rect){ox, oy, ps, 7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox, oy+6*ps, 5*ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'V':
            r = (SDL_Rect){ox,      oy, ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy, ps,   5*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+5*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+3*ps, oy+5*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+6*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'W':
            r = (SDL_Rect){ox,      oy, ps,   7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy, ps,   7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+3*ps, ps, 4*ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'N':
            r = (SDL_Rect){ox,      oy, ps,   7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy, ps,   7*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+ps,   ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+2*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+3*ps, oy+3*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        case 'X':
            r = (SDL_Rect){ox,      oy,      ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy,      ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+ps,   ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+3*ps, oy+ps,   ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+2*ps, oy+2*ps, ps, 3*ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+ps,   oy+5*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+3*ps, oy+5*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox,      oy+6*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            r = (SDL_Rect){ox+4*ps, oy+6*ps, ps, ps}; SDL_RenderFillRect(renderer, &r);
            break;
        default:
            break;
    }
    (void)BITMAPS; /* éviter warning unused */

}

/* Affiche une chaîne de caractères en bitmap */
/*
 * Utilise des rectangles SDL pour dessiner les lettres.
 * Chaque caractère est rendu comme un petit "7-segment" simplifié.
 */
static void draw_label(SDL_Renderer *renderer, int ox, int oy, const char *str)
{
    int i;
    int char_w = 5 * 2 + 2; /* Largeur d'un caractère : 5 pixels × 2 (scale) + 2px d'espacement */
    
    for (i = 0; str[i] != '\0'; i++) {
        draw_char_bitmap(renderer, ox + i * char_w, oy, str[i]);
    }

}

/* ─── Implémentation publique ────────────────────────────────────────────── */

/* Efface l'écran en noir */
/*
 * Doit être appelée au début de chaque frame.
 */
void render_clear(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

}

/* Affiche la carte (murs, pastilles, grille) */
/*
 * Effectue plusieurs passes pour bien afficher les éléments:
 * 1. Remplir les cases murs avec la couleur sombre
 * 2. Ajouter les bordures lumineuses aux murs
 * 3. Dessiner la porte de la maison des fantômes
 * 4. Afficher toutes les pastilles (petites et super)
 */
void render_map(SDL_Renderer *renderer, const Map *map, int tick)
{
    /* Super pastilles clignotent : visibles 15 frames sur 20 */
    int super_visible = (tick % 20) < 15;

    int r, c;

    /* ─── Passe 1 : remplir les murs avec fond sombre ─── */
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            SDL_Rect rect = {c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            int tile = map->tiles[r][c];

            if (tile == TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 170, 255);
                SDL_RenderFillRect(renderer, &rect);

            }
        }
    }

    /* ─── Passe 2 : ajouter les bordures lumineuses aux murs ─── */
    SDL_SetRenderDrawColor(renderer, 33, 33, 255, 255);


    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            /* Ne tracer les bordures que pour les murs */
            if (map->tiles[r][c] != TILE_WALL) continue;

            int x0 = c * TILE_SIZE;
            int y0 = r * TILE_SIZE;
            int x1 = x0 + TILE_SIZE - 1;
            int y1 = y0 + TILE_SIZE - 1;

            /*
             * Tracer les bordures seulement si le voisin n'est pas un mur
             * (pour créer l'effet de relief des bordures lumineuses)
             */
            

            /* Bordure haut */
            if (r == 0 || map->tiles[r-1][c] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x0, y0,   x1, y0);
                SDL_RenderDrawLine(renderer, x0, y0+1, x1, y0+1); /* Doublé pour épaisseur */
            }
            
            /* Bordure bas */
            if (r == MAP_ROWS-1 || map->tiles[r+1][c] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x0, y1,   x1, y1);
                SDL_RenderDrawLine(renderer, x0, y1-1, x1, y1-1);
            }
            
            /* Bordure gauche */
            if (c == 0 || map->tiles[r][c-1] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x0,   y0, x0,   y1);
                SDL_RenderDrawLine(renderer, x0+1, y0, x0+1, y1);
            }
            
            /* Bordure droite */
            if (c == MAP_COLS-1 || map->tiles[r][c+1] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x1,   y0, x1,   y1);
                SDL_RenderDrawLine(renderer, x1-1, y0, x1-1, y1);
            }
        }
    }

    /* ─── Passe 3 : porte de la maison des fantômes ─── */
    {
        SDL_SetRenderDrawColor(renderer, 255, 182, 255, 255);
        int door_y = 12 * TILE_SIZE;
        
        /* La porte s'étend horizontalement de la colonne 13 à 15 */

        SDL_RenderDrawLine(renderer, 13 * TILE_SIZE, door_y,
                                     15 * TILE_SIZE, door_y);
        SDL_RenderDrawLine(renderer, 13 * TILE_SIZE, door_y + 1,
                                     15 * TILE_SIZE, door_y + 1);
        SDL_RenderDrawLine(renderer, 13 * TILE_SIZE, door_y + 2,
                                     15 * TILE_SIZE, door_y + 2);
    }

    /* ─── Passe 4 : pastilles et super pastilles ─── */
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            int tile = map->tiles[r][c];
            int px = tile_to_px(c);
            int py = tile_to_py(r);

            if (tile == TILE_DOT) {
                SDL_SetRenderDrawColor(renderer, 255, 184, 174, 255);
                fill_circle(renderer, px, py, 2);
            } 
            else if (tile == TILE_SUPER && super_visible) {
                SDL_SetRenderDrawColor(renderer, 255, 184, 174, 255);
                fill_circle(renderer, px, py, 5);
            }

        }
    }
}

/* Affiche le joueur (Pac-Man) au centre de sa case */
/*
 * Deux animations:
 * - anim_frame = 0 : cercle avec bouche ouverte (selon direction)
 * - anim_frame = 1 : cercle complet (bouche fermée)
 */
void render_player(SDL_Renderer *renderer, const Player *player)
{
    int px = tile_to_px(player->pos.x);
    int py = tile_to_py(player->pos.y);
    int radius = (TILE_SIZE / 2) - 2;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

    if (player->anim_frame == 1 || player->dir == DIR_NONE) {

        fill_circle(renderer, px, py, radius);
        return;
    }

    /* Sinon (anim_frame = 0), dessiner un cercle avec la "bouche" ouverte */

    int x, y;
    for (y = -radius; y <= radius; y++) {
        for (x = -radius; x <= radius; x++) {
            if (x*x + y*y > radius*radius)
                continue;

            /*
             * Détection de la "bouche" :
             * Selon la direction, créer un triangle qui sort du cercle
             * La bouche est la zone QUI NE DOIT PAS ÊTRE AFFICHÉE
             */

            int in_mouth = 0;
            int half = radius / 2;
            switch (player->dir) {
                case DIR_RIGHT: in_mouth = (x > 0 && y >= -half && y <= half && y*y < x*x/2); break;
                case DIR_LEFT:  in_mouth = (x < 0 && y >= -half && y <= half && y*y < x*x/2); break;
                case DIR_UP:    in_mouth = (y < 0 && x >= -half && x <= half && x*x < y*y/2); break;
                case DIR_DOWN:  in_mouth = (y > 0 && x >= -half && x <= half && x*x < y*y/2); break;
                default: break;
            }
            
            if (!in_mouth)
                SDL_RenderDrawPoint(renderer, px + x, py + y);

        }
    }
}

/* Affiche tous les fantômes */
/*
 * Chaque fantôme est dessiné sous forme d'une forme ronde + rectangle.
 * Couleurs des 4 fantômes : rouge (Blinky), cyan (Inky), rose (Pinky), orange (Clyde).
 * États : GHOST_SCARED = bleu pâle, sinon couleur propre avec yeux blancs.
 */
void render_ghosts(SDL_Renderer *renderer, const Ghost ghosts[GHOST_COUNT])
{
    static const Uint8 GHOST_COLORS[GHOST_COUNT][3] = {
        {255,   0,   0},  /* 0 = Blinky : rouge */
        {  0, 255, 255},  /* 1 = Inky : cyan */
        {255, 184, 255},  /* 2 = Pinky : rose */
        {255, 184,  82}   /* 3 = Clyde : orange */
    };


    int i;
    for (i = 0; i < GHOST_COUNT; i++) {
        const Ghost *g = &ghosts[i];
        
        int px = tile_to_px(g->pos.x);
        int py = tile_to_py(g->pos.y);
        int radius = (TILE_SIZE / 2) - 2;

        int cid = g->color_id;
        if (cid < 0 || cid >= GHOST_COUNT) cid = 0;

        /*
         * Choisir la couleur selon l'état
         * Si effrayé (bleu) : sinon couleur propre au fantôme
         */

        if (g->state == GHOST_SCARED) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255); /* Bleu */
        } else {
            SDL_SetRenderDrawColor(renderer,
                GHOST_COLORS[cid][0],
                GHOST_COLORS[cid][1],
                GHOST_COLORS[cid][2],
                255);
        }

        /*
         * Dessiner la forme du fantôme :
         * 1. Tête : demi-cercle (radius)
         * 2. Corps : rectangle
         */
        fill_circle(renderer, px, py - 2, radius);
        SDL_Rect body = {px - radius, py - 2, radius * 2, radius + 2};
        SDL_RenderFillRect(renderer, &body);

        /*
         * Ajouter des "encoches" en bas du fantôme (effet ondulé)
         * On noircit deux zones pour créer l'illusion d'une robe ondulée
         */

        {
            int bx = px - radius;
            int by = py - 2 + radius;
            int w3 = (radius * 2) / 3;
            SDL_Rect notch1 = {bx + w3 / 4,      by + 1, w3 / 2, 3};
            SDL_Rect notch2 = {bx + w3 + w3 / 4, by + 1, w3 / 2, 3};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); /* Noir */
            SDL_RenderFillRect(renderer, &notch1);
            SDL_RenderFillRect(renderer, &notch2);
        }

        /*
         * Dessiner les yeux selon l'état
         */
        if (g->state == GHOST_SCARED) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            fill_circle(renderer, px - 3, py - 3, 2);
            fill_circle(renderer, px + 3, py - 3, 2);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            fill_circle(renderer, px - 3, py - 3, 2);
            fill_circle(renderer, px + 3, py - 3, 2);

            /*
             * Ajouter les pupilles : un petit point noir qui regarde
             * dans la direction du fantôme
             */

            int ex = 0, ey = 0;
            switch (g->dir) {
                case DIR_UP:    ey = -1; break;
                case DIR_DOWN:  ey =  1; break;
                case DIR_LEFT:  ex = -1; break;
                case DIR_RIGHT: ex =  1; break;
                default: break;
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
            fill_circle(renderer, px - 3 + ex, py - 3 + ey, 1);
            fill_circle(renderer, px + 3 + ex, py - 3 + ey, 1);

        }
    }
}

/*
 * Rendu d'un chiffre (0-9) en style "7 segments".
 * Utilise 7 segments : haut, haut-droite, bas-droite, bas, bas-gauche, haut-gauche, milieu
 * Chaque chiffre 0-9 allume certains segments d'après un motif binaire.
 */

/*
 * Affiche un chiffre 0-9 avec 7 segments
 * Représentation binaire : bit 6=a(haut) 5=b(top-right)... 0=g(milieu)
 */
static void draw_digit(SDL_Renderer *renderer, int ox, int oy, int d)
{
    /* Table de motifs pour chaque chiffre 0-9 */
    static const unsigned char SEG[10] = {

        0x7E, /* 0 : a b c d e f     */
        0x30, /* 1 :   b c           */
        0x6D, /* 2 : a b   d e   g   */
        0x79, /* 3 : a b c d       g */
        0x33, /* 4 :   b c     f g   */
        0x5B, /* 5 : a   c d   f g   */
        0x5F, /* 6 : a   c d e f g   */
        0x70, /* 7 : a b c           */
        0x7F, /* 8 : a b c d e f g   */
        0x7B, /* 9 : a b c d   f g   */
    };

    if (d < 0 || d > 9) return;
    unsigned char s = SEG[d];

    SDL_Rect r;
    
    if (s & 0x40) { r = (SDL_Rect){ox+1, oy,    6, 2}; SDL_RenderFillRect(renderer, &r); } /* a */
    if (s & 0x20) { r = (SDL_Rect){ox+7, oy+1,  2, 5}; SDL_RenderFillRect(renderer, &r); } /* b */
    if (s & 0x10) { r = (SDL_Rect){ox+7, oy+7,  2, 5}; SDL_RenderFillRect(renderer, &r); } /* c */
    if (s & 0x08) { r = (SDL_Rect){ox+1, oy+12, 6, 2}; SDL_RenderFillRect(renderer, &r); } /* d */
    if (s & 0x04) { r = (SDL_Rect){ox,   oy+7,  2, 5}; SDL_RenderFillRect(renderer, &r); } /* e */
    if (s & 0x02) { r = (SDL_Rect){ox,   oy+1,  2, 5}; SDL_RenderFillRect(renderer, &r); } /* f */
    if (s & 0x01) { r = (SDL_Rect){ox+1, oy+6,  6, 2}; SDL_RenderFillRect(renderer, &r); } /* g */

}

/*
 * Affiche un entier en utilisant draw_digit
 * Affiche le nombre sur N chiffres (left-padded avec des 0 si nécessaire).
 */
static void draw_number(SDL_Renderer *renderer, int ox, int oy, int value, int ndigits)
{
    int digits[8];
    int i;
    
    for (i = ndigits - 1; i >= 0; i--) {
        digits[i] = value % 10;
        value /= 10;
    }
    
    for (i = 0; i < ndigits; i++)
        draw_digit(renderer, ox + i * 12, oy, digits[i]); /* Espacé de 12px */

}

/* Affiche le HUD (score, niveau, vies) */
/*
 * Zone en bas de l'écran (60px de haut) divisée en 3 sections.
 * Utilise des 7-segments pour les chiffres et des bitmaps pour les labels.
 */
void render_hud(SDL_Renderer *renderer, const Player *player, int level, int high_score)
{
    int hud_y = MAP_ROWS * TILE_SIZE;
    int i;

    SDL_Rect bg = {0, hud_y, WINDOW_WIDTH, HUD_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_Rect sep = {0, hud_y, WINDOW_WIDTH, 1};
    SDL_RenderFillRect(renderer, &sep);

    /*
     * Layout : WINDOW_WIDTH = 560px
     * Section 1 (SCORE)  : x=0...187
     * Section 2 (LEVEL)  : x=187...373
     * Section 3 (LIVES)  : x=373...560
     */


    int label_y  = hud_y + 2;
    int number_y = hud_y + 22;
    int digit_h  = 14;
    int cy = number_y + digit_h / 2;

    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
    draw_label(renderer, 15, label_y, "SCORE");
    draw_number(renderer, 20, number_y, player->score, 6);

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, 187, hud_y + 5, 187, hud_y + HUD_HEIGHT - 5);

    SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
    draw_label(renderer, 210, label_y, "LVL");
    draw_digit(renderer, 255, number_y, level % 10);

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, 373, hud_y + 5, 373, hud_y + HUD_HEIGHT - 5);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    draw_label(renderer, 390, label_y, "LIVES");

    /*
     * Afficher les vies sous forme de petits cercles jaunes
     * Limité à 5 vies maximum (sinon trop d'affichage)
     */

    for (i = 0; i < player->lives && i < 5; i++) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        fill_circle(renderer,
                    530 - i * 22,  /* Ancré à droite, espacé de 22px */
                    cy,            /* Centre Y du HUD */
                    7);            /* Rayon de 7px */
    }

}

/* Affiche un message centré à l'écran */
/*
 * Utilisée pour afficher les messages "GAME OVER", "YOU WIN!", "SCORE", etc.
 * Affiche une boîte avec bordure blanche et texte bitmap centré.
 */
void render_overlay_text(SDL_Renderer *renderer, const char *label)
{
    /*
     * Boîte centrée à l'écran
     * Dimensions : 160×40 pixels
     */
    SDL_Rect box = {
        WINDOW_WIDTH / 2 - 80,
        WINDOW_HEIGHT / 2 - 20,
        160, 40
    };

    /*
     * Choisir la couleur selon le contenu du message
     * Les couleurs ont une transparence (alpha=200)
     */

    if (strcmp(label, "GAME OVER") == 0)
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 200);
    else if (strcmp(label, "YOU WIN!") == 0)
        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 200);
    else
        SDL_SetRenderDrawColor(renderer, 0, 0, 120, 200);

    SDL_RenderFillRect(renderer, &box);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &box);

    /*
     * Afficher le texte bitmap centré à l'intérieur
     * Calculer la largeur totale du texte pour le centrer
     */

    int len = (int)strlen(label);
    int char_w = 5 * 2 + 2;
    int total_w = len * char_w;
    int bx = WINDOW_WIDTH / 2 - total_w / 2;
    int by = WINDOW_HEIGHT / 2 - 7;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int i;
    for (i = 0; i < len; i++) {
        if (label[i] != ' ' && label[i] != '!')
            draw_char_bitmap(renderer, bx + i * char_w, by, label[i]);
    }

}

/*
 * render_present  – Présente le rendu à l'écran (swap du front/back buffer)
 * 
 * Doit être appelée à la fin de chaque frame pour afficher le rendu.
 * Avec SDL_RENDERER_PRESENTVSYNC, cela attend la synchro verticale (vsync).
 */
void render_present(SDL_Renderer *renderer)
{
    SDL_RenderPresent(renderer);
}
