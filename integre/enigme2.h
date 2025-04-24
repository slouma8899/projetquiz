#ifndef ENIGME2_H
#define ENIGME2_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

extern int SCREEN_W;
extern int SCREEN_H;
#define TILE_SIZE 100

extern Mix_Chunk *flipSound;
extern Mix_Chunk *matchSound;
extern Mix_Chunk *wrongSound;
extern Mix_Chunk *winSound;
extern Mix_Chunk *loseSound;
extern TTF_Font *gFont;

Uint32 interpolateColor(Uint32 start, Uint32 end, float ratio);
SDL_Surface* CreateDummySurfaceDynamic(int tile_size);

typedef struct {
    int grid_size;         // 2 for 2×2, 4 for 4×4.
    int total_pairs;       // (grid_size * grid_size) / 2
    SDL_Surface ***tiles;  // 2D array [grid_size][grid_size]
    SDL_Surface **images;  // Array of images; size: total_pairs
    SDL_Rect **positions;  // Each tile's position
    int **flipped;         // 0 or 1; tile face up?
    int selected[2];       // Flattened indices; -1 means none selected.
    int matches;
    Uint32 start_time;
    int total_time;        // In seconds; level dependent.
    int time_left;
    int game_over;
    SDL_Surface *dummy;    // (unused)
    int score;             // Calculated as game->matches * game->time_left
    int difficulty;        // 1 = Easy, 2 = Hard, 3 = Extreme.
} MemoryGame;

void initialiser_enigme(MemoryGame *game, const char *img_dir, int grid_size, int difficulty);
void Memory_HandleEvent(MemoryGame *game, SDL_Event *ev);
void Memory_Update(MemoryGame *game);
void Memory_Render(MemoryGame *game, SDL_Surface *screen);
void Memory_Cleanup(MemoryGame *game);

#endif // ENIGME2_H
