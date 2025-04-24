#include "enigme2.h"

int SCREEN_W = 800;
int SCREEN_H = 600;

Uint32 interpolateColor(Uint32 start, Uint32 end, float ratio) {
    int r1 = (start >> 16) & 0xFF, g1 = (start >> 8) & 0xFF, b1 = start & 0xFF;
    int r2 = (end >> 16) & 0xFF, g2 = (end >> 8) & 0xFF, b2 = end & 0xFF;
    int r = r1 + (int)((r2 - r1) * ratio);
    int g = g1 + (int)((g2 - g1) * ratio);
    int b = b1 + (int)((b2 - b1) * ratio);
    return (r << 16) | (g << 8) | b;
}

SDL_Surface* CreateDummySurfaceDynamic(int tile_size) {
    SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, tile_size, tile_size, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (surf)
        SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, 255, 255, 0));
    return surf;
}

void initialiser_enigme(MemoryGame *game, const char *img_dir, int grid_size, int difficulty) {
    srand(time(NULL));
    game->grid_size = grid_size;
    game->difficulty = difficulty;
    int spacing = 20;
    int tile_size = TILE_SIZE;
    
    int total_cells = grid_size * grid_size;
    game->total_pairs = total_cells / 2;
    game->matches = 0;
    game->start_time = SDL_GetTicks();
    game->total_time = (difficulty == 3) ? 20 : 30;
    game->time_left = game->total_time;
    game->game_over = 0;
    game->selected[0] = -1;
    game->selected[1] = -1;
    game->dummy = NULL;
    game->score = 0;
    
    // Allocate arrays.
    game->tiles = malloc(grid_size * sizeof(SDL_Surface **));
    game->positions = malloc(grid_size * sizeof(SDL_Rect *));
    game->flipped = malloc(grid_size * sizeof(int *));
    for (int i = 0; i < grid_size; i++) {
        game->tiles[i] = malloc(grid_size * sizeof(SDL_Surface *));
        game->positions[i] = malloc(grid_size * sizeof(SDL_Rect));
        game->flipped[i] = malloc(grid_size * sizeof(int));
        for (int j = 0; j < grid_size; j++)
            game->flipped[i][j] = 0;
    }
    
    // Load images.
    game->images = malloc(game->total_pairs * sizeof(SDL_Surface *));
    for (int i = 0; i < game->total_pairs; i++) {
        char path[256];
        snprintf(path, sizeof(path), "%s/images/%d.jpg", img_dir, i + 1);
        FILE *fp = fopen(path, "r");
        if (!fp) {
            printf("Warning: file not found: %s. Using dummy image.\n", path);
            game->images[i] = CreateDummySurfaceDynamic(tile_size);
        } else {
            fclose(fp);
            SDL_Surface *original = IMG_Load(path);
            if (!original) {
                printf("Error loading image: %s\n", path);
                exit(1);
            }
            game->images[i] = rotozoomSurface(original, 0, (double)tile_size / original->w, 1);
            SDL_FreeSurface(original);
            if (!game->images[i]) {
                printf("Error scaling image: %s\n", path);
                exit(1);
            }
        }
    }
    
    // Create and shuffle pairs.
    int *pairs = malloc(total_cells * sizeof(int));
    for (int i = 0; i < game->total_pairs; i++) {
        pairs[i * 2] = i;
        pairs[i * 2 + 1] = i;
    }
    for (int i = 0; i < total_cells; i++) {
        int j = rand() % total_cells;
        int temp = pairs[i];
        pairs[i] = pairs[j];
        pairs[j] = temp;
    }
    
    int start_x = (SCREEN_W - (grid_size * (tile_size + spacing))) / 2;
    int start_y = (SCREEN_H - (grid_size * (tile_size + spacing))) / 2;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            int idx = pairs[i * grid_size + j];
            game->tiles[i][j] = game->images[idx];
            game->positions[i][j] = (SDL_Rect){
                start_x + j * (tile_size + spacing),
                start_y + i * (tile_size + spacing),
                tile_size,
                tile_size
            };
        }
    }
    free(pairs);
}

void Memory_HandleEvent(MemoryGame *game, SDL_Event *ev) {
    if (game->game_over)
        return;
    if (ev->type == SDL_MOUSEBUTTONDOWN) {
        int x = ev->button.x, y = ev->button.y;
        for (int i = 0; i < game->grid_size; i++) {
            for (int j = 0; j < game->grid_size; j++) {
                SDL_Rect pos = game->positions[i][j];
                if (x >= pos.x && x <= pos.x + pos.w &&
                    y >= pos.y && y <= pos.y + pos.h &&
                    !game->flipped[i][j])
                {
                    int cell_index = i * game->grid_size + j;
                    if (game->selected[0] == -1) {
                        game->selected[0] = cell_index;
                        game->flipped[i][j] = 1;
                        Mix_PlayChannel(-1, flipSound, 0);
                    } else if (game->selected[1] == -1) {
                        game->selected[1] = cell_index;
                        game->flipped[i][j] = 1;
                        Mix_PlayChannel(-1, flipSound, 0);
                    }
                }
            }
        }
    }
}

void Memory_Update(MemoryGame *game) {
    if (game->game_over)
        return;
    int reveal_delay = (game->grid_size == 2) ? 2000 : 1000;
    
    static Uint32 mismatch_time = 0;
    if (game->selected[0] != -1 && game->selected[1] != -1) {
        int i1 = game->selected[0] / game->grid_size;
        int j1 = game->selected[0] % game->grid_size;
        int i2 = game->selected[1] / game->grid_size;
        int j2 = game->selected[1] % game->grid_size;
        if (game->tiles[i1][j1] == game->tiles[i2][j2]) {
            game->matches++;
            game->selected[0] = -1;
            game->selected[1] = -1;
            Mix_PlayChannel(-1, matchSound, 0);
            if (game->matches == game->total_pairs) {
                game->game_over = 1;
                Mix_PlayChannel(-1, winSound, 0);
            }
        } else {
            if (mismatch_time == 0)
                mismatch_time = SDL_GetTicks();
            if (SDL_GetTicks() - mismatch_time > reveal_delay) {
                game->flipped[i1][j1] = 0;
                game->flipped[i2][j2] = 0;
                game->selected[0] = -1;
                game->selected[1] = -1;
                mismatch_time = 0;
                Mix_PlayChannel(-1, wrongSound, 0);
            }
        }
    } else {
        mismatch_time = 0;
    }
    
    Uint32 current_time = SDL_GetTicks();
    game->time_left = game->total_time - (current_time - game->start_time) / 1000;
    if (game->time_left <= 0) {
        if (!game->game_over) {
            game->time_left = 0;
            game->game_over = 1;
            if (game->matches != game->total_pairs)
                Mix_PlayChannel(-1, loseSound, 0);
        }
    }
    if (game->game_over)
        game->score = game->matches * game->time_left;
}

void Memory_Render(MemoryGame *game, SDL_Surface *screen) {
    Uint32 bgColor;
    if (game->difficulty == 1)
        bgColor = SDL_MapRGB(screen->format, 70, 130, 180);
    else if (game->difficulty == 2)
        bgColor = SDL_MapRGB(screen->format, 128, 0, 128);
    else
        bgColor = SDL_MapRGB(screen->format, 139, 0, 0);
    SDL_FillRect(screen, NULL, bgColor);
    
    Uint32 current_time = SDL_GetTicks();
    int preview = (game->grid_size > 2 && ((current_time - game->start_time) < 3000));
    for (int i = 0; i < game->grid_size; i++) {
        for (int j = 0; j < game->grid_size; j++) {
            SDL_Rect pos = game->positions[i][j];
            if (preview)
                SDL_BlitSurface(game->tiles[i][j], NULL, screen, &pos);
            else {
                if (game->flipped[i][j])
                    SDL_BlitSurface(game->tiles[i][j], NULL, screen, &pos);
                else {
                    SDL_FillRect(screen, &pos, SDL_MapRGB(screen->format, 100, 100, 150));
                    rectangleColor(screen, pos.x, pos.y, pos.x + pos.w, pos.y + pos.h, 0xFFFFFFFF);
                }
            }
        }
    }
    
    int bar_width = 400, bar_height = 25;
    int bar_x = (SCREEN_W - bar_width) / 2, bar_y = 20;
    boxColor(screen, bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, 0x505050FF);
    int current_width = (game->time_left / (float)game->total_time) * bar_width;
    Uint32 color = interpolateColor(0x00FF00FF, 0xFF0000FF, 1 - (game->time_left/(float)game->total_time));
    boxColor(screen, bar_x, bar_y, bar_x + current_width, bar_y + bar_height, color);
    
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Pairs: %d/%d", game->matches, game->total_pairs);
    stringColor(screen, 10, 30, buffer, 0xFFFFFFFF);
    snprintf(buffer, sizeof(buffer), "Time: %d", game->time_left);
    stringColor(screen, 10, 10, buffer, 0xFFFFFFFF);
    
    const char *levelLabel;
    if (game->difficulty == 1)
        levelLabel = "Level 1 - Easy";
    else if (game->difficulty == 2)
        levelLabel = "Level 2 - Hard";
    else
        levelLabel = "Level 3 - Extreme";
    stringColor(screen, SCREEN_W/2 - 50, 5, levelLabel, 0xFFFFFFFF);
    
    if (game->time_left < 5)
        boxColor(screen, 0, 0, SCREEN_W, SCREEN_H, 0x88000000);
    
    if (game->game_over) {
        SDL_Surface *overlay = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W, 150, 32,
            screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
        if (overlay) {
            SDL_FillRect(overlay, NULL, SDL_MapRGBA(overlay->format, 255, 255, 255, 128));
            SDL_SetAlpha(overlay, SDL_SRCALPHA, 128);
            SDL_Rect overlayPos = {0, SCREEN_H - 150, SCREEN_W, 150};
            SDL_BlitSurface(overlay, NULL, screen, &overlayPos);
            SDL_FreeSurface(overlay);
        }
        
        const char *msg = (game->matches == game->total_pairs) ? "YOU WON" : "YOU LOST";
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface *msgSurface = TTF_RenderText_Blended(gFont, msg, textColor);
        if (msgSurface) {
            SDL_Rect msgRect;
            msgRect.x = (SCREEN_W - msgSurface->w) / 2;
            msgRect.y = SCREEN_H - 150 + 20;
            SDL_BlitSurface(msgSurface, NULL, screen, &msgRect);
            SDL_FreeSurface(msgSurface);
        }
        
        char scoreBuffer[50];
        snprintf(scoreBuffer, sizeof(scoreBuffer), "Your Score: %d", game->score);
        SDL_Surface *scoreSurface = TTF_RenderText_Blended(gFont, scoreBuffer, textColor);
        if (scoreSurface) {
            SDL_Rect scoreRect;
            scoreRect.x = (SCREEN_W - scoreSurface->w) / 2;
            scoreRect.y = SCREEN_H - 150 + 80;
            SDL_BlitSurface(scoreSurface, NULL, screen, &scoreRect);
            SDL_FreeSurface(scoreSurface);
        }
    }
}

void Memory_Cleanup(MemoryGame *game) {
    for (int i = 0; i < game->total_pairs; i++)
        SDL_FreeSurface(game->images[i]);
    free(game->images);
    for (int i = 0; i < game->grid_size; i++) {
        free(game->tiles[i]);
        free(game->positions[i]);
        free(game->flipped[i]);
    }
    free(game->tiles);
    free(game->positions);
    free(game->flipped);
    if (game->dummy)
        SDL_FreeSurface(game->dummy);
}
