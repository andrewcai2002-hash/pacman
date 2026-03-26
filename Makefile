# ─────────────────────────────────────────────────────────────────────────────
# Makefile – Projet Pacman en C avec SDL2
# Usage :
#   make          → compile le projet
#   make run      → compile puis lance le jeu
#   make clean    → supprime les objets et l'exécutable
#   make help     → affiche cette aide
# ─────────────────────────────────────────────────────────────────────────────

# ── Compilateur et options ───────────────────────────────────────────────────
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g \
          $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lm

# ── Dossiers ─────────────────────────────────────────────────────────────────
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# ── Fichiers sources et objets ───────────────────────────────────────────────
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# ── Nom de l'exécutable ──────────────────────────────────────────────────────
TARGET = pacman

# ─────────────────────────────────────────────────────────────────────────────
# Règles
# ─────────────────────────────────────────────────────────────────────────────

.PHONY: all run clean help

all: $(TARGET)

## Lien final
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✓ Compilation réussie → ./$(TARGET)"

## Compilation de chaque fichier source
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

## Création du dossier obj si absent
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

## Lancer le jeu après compilation
run: all
	./$(TARGET)

## Nettoyage
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "✓ Nettoyage effectué"

## Aide
help:
	@echo "Commandes disponibles :"
	@echo "  make        → compile le projet"
	@echo "  make run    → compile et lance le jeu"
	@echo "  make clean  → nettoie les fichiers générés"
