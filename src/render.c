/*
 * src/render.c
 * Implémentation du module rendu.
 * Tout est dessiné avec des primitives SDL2 (rectangles, cercles approchés).
 * Pas de SDL_ttf dans le MVP : le score est représenté par un indicateur visuel
 * simple, et les messages d'état par un rectangle coloré.
 *
 * NOTE : Pour ajouter de vraies polices, il suffit d'intégrer SDL2_ttf et de
 * remplacer render_hud / render_overlay_text.
 */

#include <string.h>
#include "../include/render.h"

/* ─── Utilitaire : dessiner un cercle rempli ─────────────────────────────── */
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
static int tile_to_px(int col) { return col * TILE_SIZE + TILE_SIZE / 2; }
static int tile_to_py(int row) { return row * TILE_SIZE + TILE_SIZE / 2; }

/* ─── Implémentation publique ────────────────────────────────────────────── */

void render_clear(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void render_map(SDL_Renderer *renderer, const Map *map, int tick)
{
    int super_visible = (tick % 20) < 15;
    int r, c;

    /* ── Passe 1 : fond de toutes les cases ── */
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            SDL_Rect rect = {c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            int tile = map->tiles[r][c];

            if (tile == TILE_WALL) {
                /* Remplissage bleu foncé */
                SDL_SetRenderDrawColor(renderer, 0, 0, 170, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    /* ── Passe 2 : bordures lumineuses des murs (style original) ──
     * On dessine une ligne cyan brillante sur chaque bord d'un mur
     * qui est adjacent à une case NON-mur. */
    SDL_SetRenderDrawColor(renderer, 33, 33, 255, 255);  /* Bleu vif */

    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            if (map->tiles[r][c] != TILE_WALL) continue;

            int x0 = c * TILE_SIZE;
            int y0 = r * TILE_SIZE;
            int x1 = x0 + TILE_SIZE - 1;
            int y1 = y0 + TILE_SIZE - 1;

            /* Bord haut */
            if (r == 0 || map->tiles[r-1][c] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x0, y0,   x1, y0);
                SDL_RenderDrawLine(renderer, x0, y0+1, x1, y0+1);
            }
            /* Bord bas */
            if (r == MAP_ROWS-1 || map->tiles[r+1][c] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x0, y1,   x1, y1);
                SDL_RenderDrawLine(renderer, x0, y1-1, x1, y1-1);
            }
            /* Bord gauche */
            if (c == 0 || map->tiles[r][c-1] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x0,   y0, x0,   y1);
                SDL_RenderDrawLine(renderer, x0+1, y0, x0+1, y1);
            }
            /* Bord droit */
            if (c == MAP_COLS-1 || map->tiles[r][c+1] != TILE_WALL) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 255, 255);
                SDL_RenderDrawLine(renderer, x1,   y0, x1,   y1);
                SDL_RenderDrawLine(renderer, x1-1, y0, x1-1, y1);
            }
        }
    }

    /* ── Passe 3 : porte de la maison des fantômes (ligne rose) ── */
    /* La porte est à l'entrée de la maison (y=12, col=13-14) */
    {
        SDL_SetRenderDrawColor(renderer, 255, 182, 255, 255);
        int door_y = 12 * TILE_SIZE;
        SDL_RenderDrawLine(renderer, 13 * TILE_SIZE, door_y,
                                     15 * TILE_SIZE, door_y);
        SDL_RenderDrawLine(renderer, 13 * TILE_SIZE, door_y + 1,
                                     15 * TILE_SIZE, door_y + 1);
        SDL_RenderDrawLine(renderer, 13 * TILE_SIZE, door_y + 2,
                                     15 * TILE_SIZE, door_y + 2);
    }

    /* ── Passe 4 : pastilles et super pastilles ── */
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            int tile = map->tiles[r][c];
            int px = tile_to_px(c);
            int py = tile_to_py(r);

            if (tile == TILE_DOT) {
                SDL_SetRenderDrawColor(renderer, 255, 184, 174, 255);
                fill_circle(renderer, px, py, 2);
            } else if (tile == TILE_SUPER && super_visible) {
                SDL_SetRenderDrawColor(renderer, 255, 184, 174, 255);
                fill_circle(renderer, px, py, 5);
            }
        }
    }
}

void render_player(SDL_Renderer *renderer, const Player *player)
{
    int px = tile_to_px(player->pos.x);
    int py = tile_to_py(player->pos.y);
    int radius = (TILE_SIZE / 2) - 2;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

    /* Bouche fermée : cercle plein */
    if (player->anim_frame == 1 || player->dir == DIR_NONE) {
        fill_circle(renderer, px, py, radius);
        return;
    }

    /* Bouche ouverte : on dessine le cercle en excluant le secteur de la bouche.
     * On pixelise manuellement : pour chaque pixel du carré englobant,
     * on vérifie s'il est dans le cercle ET hors du triangle de la bouche. */
    int x, y;
    for (y = -radius; y <= radius; y++) {
        for (x = -radius; x <= radius; x++) {
            if (x*x + y*y > radius*radius)
                continue;

            /* Vecteur bouche selon la direction */
            int in_mouth = 0;
            int half = radius / 2; /* Demi-angle de la bouche */
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

void render_ghosts(SDL_Renderer *renderer, const Ghost ghosts[GHOST_COUNT])
{
    static const Uint8 GHOST_COLORS[GHOST_COUNT][3] = {
        {255,   0,   0},  /* Ghost 0 – rouge       (Blinky)  */
        {  0, 255, 255},  /* Ghost 1 – bleu clair (Inky)    */
        {255, 184, 255},  /* Ghost 2 – rose       (Pinky)   */
        {255, 184,  82}   /* Ghost 3 – orange     (Clyde)   */
    };

    int i;
    for (i = 0; i < GHOST_COUNT; i++) {
        const Ghost *g = &ghosts[i];
        int px = tile_to_px(g->pos.x);
        int py = tile_to_py(g->pos.y);
        int radius = (TILE_SIZE / 2) - 2;

        /* Sécurité : color_id toujours dans le tableau */
        int cid = g->color_id;
        if (cid < 0 || cid >= GHOST_COUNT) cid = 0;

        if (g->state == GHOST_SCARED) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
        } else {
            SDL_SetRenderDrawColor(renderer,
                GHOST_COLORS[cid][0],
                GHOST_COLORS[cid][1],
                GHOST_COLORS[cid][2],
                255);
        }

        /* Corps : demi-cercle supérieur + rectangle inférieur */
        fill_circle(renderer, px, py - 2, radius);
        SDL_Rect body = {px - radius, py - 2, radius * 2, radius + 2};
        SDL_RenderFillRect(renderer, &body);

        /* Bas du corps en dents (3 bosses) pour la forme fantôme */
        {
            int bx = px - radius;
            int by = py - 2 + radius;
            int w3 = (radius * 2) / 3;
            SDL_Rect notch1 = {bx + w3 / 4,     by + 1, w3 / 2, 3};
            SDL_Rect notch2 = {bx + w3 + w3 / 4, by + 1, w3 / 2, 3};
            /* On dessine le fond noir pour créer les dents */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &notch1);
            SDL_RenderFillRect(renderer, &notch2);
        }

        /* Yeux */
        if (g->state == GHOST_SCARED) {
            /* Yeux blancs simples en mode peur */
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            fill_circle(renderer, px - 3, py - 3, 2);
            fill_circle(renderer, px + 3, py - 3, 2);
        } else {
            /* Yeux blancs + pupilles bleues orientées selon la direction */
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            fill_circle(renderer, px - 3, py - 3, 2);
            fill_circle(renderer, px + 3, py - 3, 2);

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
 * Rendu d'un chiffre (0-9) en style "7 segments" minimaliste.
 * Chaque segment est un rectangle de 2×6 ou 6×2 pixels.
 * ox, oy = coin supérieur gauche du chiffre (largeur 9px, hauteur 14px)
 */
static void draw_digit(SDL_Renderer *renderer, int ox, int oy, int d)
{
    /*
     * Segments : a=haut, b=haut-droite, c=bas-droite,
     *            d=bas, e=bas-gauche, f=haut-gauche, g=milieu
     * Table de vérité pour 0..9
     */
    static const unsigned char SEG[10] = {
        0x77, /* 0 : a b c d e f   */
        0x12, /* 1 :   b c         */
        0x6D, /* 2 : a b   d e   g */
        0x79, /* 3 : a b c d     g */
        0x1B, /* 4 :   b c   f g   */
        0x5B, /* 5 : a   c d   f g */
        0x5F, /* 6 : a   c d e f g */
        0x72, /* 7 : a b c         */
        0x7F, /* 8 : a b c d e f g */
        0x7B, /* 9 : a b c d   f g */
    };

    if (d < 0 || d > 9) return;
    unsigned char s = SEG[d];

    SDL_Rect r;
    /* a – haut */
    if (s & 0x40) { r = (SDL_Rect){ox+1, oy,    6, 2}; SDL_RenderFillRect(renderer, &r); }
    /* b – haut droite */
    if (s & 0x20) { r = (SDL_Rect){ox+7, oy+1,  2, 5}; SDL_RenderFillRect(renderer, &r); }
    /* c – bas droite */
    if (s & 0x10) { r = (SDL_Rect){ox+7, oy+7,  2, 5}; SDL_RenderFillRect(renderer, &r); }
    /* d – bas */
    if (s & 0x08) { r = (SDL_Rect){ox+1, oy+12, 6, 2}; SDL_RenderFillRect(renderer, &r); }
    /* e – bas gauche */
    if (s & 0x04) { r = (SDL_Rect){ox,   oy+7,  2, 5}; SDL_RenderFillRect(renderer, &r); }
    /* f – haut gauche */
    if (s & 0x02) { r = (SDL_Rect){ox,   oy+1,  2, 5}; SDL_RenderFillRect(renderer, &r); }
    /* g – milieu */
    if (s & 0x01) { r = (SDL_Rect){ox+1, oy+6,  6, 2}; SDL_RenderFillRect(renderer, &r); }
}

/* Affiche un entier sur N chiffres à la position (ox, oy) */
static void draw_number(SDL_Renderer *renderer, int ox, int oy, int value, int ndigits)
{
    int digits[8];
    int i;
    for (i = ndigits - 1; i >= 0; i--) {
        digits[i] = value % 10;
        value /= 10;
    }
    for (i = 0; i < ndigits; i++)
        draw_digit(renderer, ox + i * 12, oy, digits[i]);  /* Espacement augmenté de 11 à 12 */
}

void render_hud(SDL_Renderer *renderer, const Player *player, int level, int high_score)
{
    int hud_y = MAP_ROWS * TILE_SIZE;
    int i;

    /* Fond HUD */
    SDL_Rect bg = {0, hud_y, WINDOW_WIDTH, HUD_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &bg);

    /* Ligne de séparation */
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_Rect sep = {0, hud_y, WINDOW_WIDTH, 1};
    SDL_RenderFillRect(renderer, &sep);

    int cy = hud_y + (HUD_HEIGHT - 14) / 2; /* Centrage vertical des chiffres */
    int label_y = hud_y + 8;  /* Position des labels au-dessus */

    /* ── Section SCORE (à gauche) ── */
    /* Label "SCORE" en petits carrés */
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
    for (i = 0; i < 5; i++) {
        SDL_Rect label_box = {10 + i * 5, label_y, 3, 3};
        SDL_RenderFillRect(renderer, &label_box);
    }
    /* Score sur 6 chiffres */
    draw_number(renderer, 10, cy, player->score, 6);

    /* ── Séparateur vertical ── */
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 2 - 55, hud_y + 5, WINDOW_WIDTH / 2 - 55, hud_y + HUD_HEIGHT - 5);

    /* ── Section HIGH SCORE (centre-gauche) ── */
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    /* Label "HIGH" en petits carrés */
    for (i = 0; i < 4; i++) {
        SDL_Rect label_box = {WINDOW_WIDTH / 2 - 48 + i * 5, label_y, 3, 3};
        SDL_RenderFillRect(renderer, &label_box);
    }
    /* High score sur 6 chiffres */
    draw_number(renderer, WINDOW_WIDTH / 2 - 40, cy, high_score, 6);

    /* ── Séparateur vertical ── */
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 2 + 55, hud_y + 5, WINDOW_WIDTH / 2 + 55, hud_y + HUD_HEIGHT - 5);

    /* ── Section LEVEL (centre-droit) ── */
    SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
    /* Label "LEVEL" en petits carrés */
    for (i = 0; i < 5; i++) {
        SDL_Rect label_box = {WINDOW_WIDTH / 2 + 65 + i * 5, label_y, 3, 3};
        SDL_RenderFillRect(renderer, &label_box);
    }
    /* Niveau (1 chiffre centré) */
    draw_digit(renderer, WINDOW_WIDTH / 2 + 75, cy, level);

    /* ── Séparateur vertical ── */
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawLine(renderer, WINDOW_WIDTH - 65, hud_y + 5, WINDOW_WIDTH - 65, hud_y + HUD_HEIGHT - 5);

    /* ── Section LIVES (à droite) ── */
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    /* Label "LIVES" en petits carrés */
    for (i = 0; i < 5; i++) {
        SDL_Rect label_box = {WINDOW_WIDTH - 60 + i * 5, label_y, 3, 3};
        SDL_RenderFillRect(renderer, &label_box);
    }
    /* Cercles pour indiquer les vies */
    for (i = 0; i < player->lives; i++) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        fill_circle(renderer,
                    WINDOW_WIDTH - 85 - i * 20,
                    hud_y + HUD_HEIGHT / 2,
                    7);
    }
}


void render_overlay_text(SDL_Renderer *renderer, const char *label)
{
    /* Fond semi-transparent centré */
    SDL_Rect box = {
        WINDOW_WIDTH / 2 - 80,
        WINDOW_HEIGHT / 2 - 20,
        160, 40
    };

    /* Choisir la couleur selon le message */
    if (strcmp(label, "GAME OVER") == 0)
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 200);
    else if (strcmp(label, "YOU WIN!") == 0)
        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 200);
    else
        SDL_SetRenderDrawColor(renderer, 0, 0, 120, 200);

    SDL_RenderFillRect(renderer, &box);

    /* Bordure */
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &box);

    /* Représentation textuelle simplifiée par des traits colorés */
    /* (Sans SDL_ttf, on ne peut pas afficher de vrais caractères facilement) */
    /* On dessine un motif distinctif en blocs selon le type de message */
    int bw = 8, bh = 14, gap = 4;
    int len = (int)strlen(label);
    int total_w = len * (bw + gap);
    int bx = WINDOW_WIDTH / 2 - total_w / 2;
    int by = WINDOW_HEIGHT / 2 - bh / 2;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int i;
    for (i = 0; i < len; i++) {
        if (label[i] != ' ') {
            SDL_Rect ch = {bx + i * (bw + gap), by, bw, bh};
            SDL_RenderFillRect(renderer, &ch);
        }
    }
}

void render_present(SDL_Renderer *renderer)
{
    SDL_RenderPresent(renderer);
}