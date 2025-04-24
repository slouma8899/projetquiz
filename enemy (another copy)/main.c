#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "enemy.h"

// Draw a health bar on the screen to represent an entity's health
void draw_health_bar(SDL_Surface *screen, int health, int max_health, int x, int y, int w, int h) {
    SDL_Rect bg_rect = {x, y, w, h}; // Background rectangle for the health bar
    SDL_FillRect(screen, &bg_rect, SDL_MapRGB(screen->format, 0, 0, 0)); // Fill with black as the background
    
    float percentage = (health <= 0) ? 0 : (float)health / max_health; // Calculate the health percentage
    int current_width = (int)(percentage * (w - 2)); // Width of the health bar based on the percentage
    
    Uint32 color;
    if (percentage > 0.5) color = SDL_MapRGB(screen->format, 0, 255, 0); // Green if health > 50%
    else if (percentage > 0.25) color = SDL_MapRGB(screen->format, 255, 255, 0); // Yellow if health > 25%
    else color = SDL_MapRGB(screen->format, 255, 0, 0); // Red if health <= 25%
    
    SDL_Rect health_rect = {x + 1, y + 1, current_width, h - 2}; // Rectangle for the actual health bar
    SDL_FillRect(screen, &health_rect, color); // Fill with the appropriate color
}

// Initialize a coin with a simple appearance (a gold square with a black outline)
void initCoin(Coin *coin) {
    coin->img = SDL_CreateRGBSurface(SDL_SWSURFACE, 20, 20, 32, 0, 0, 0, 0); // Create a 20x20 surface for the coin
    if (coin->img == NULL) {
        printf("Failed to create fallback coin surface: %s\n", SDL_GetError());
        return;
    }
    SDL_FillRect(coin->img, NULL, SDL_MapRGB(coin->img->format, 255, 215, 0)); // Fill with gold color
    SDL_SetColorKey(coin->img, SDL_SRCCOLORKEY, SDL_MapRGB(coin->img->format, 0, 0, 0)); // Set black as the transparent color
    SDL_Rect outline = {0, 0, 20, 20}; // Outline rectangle
    SDL_FillRect(coin->img, &outline, SDL_MapRGB(coin->img->format, 0, 0, 0)); // Draw black outline
    SDL_Rect inner = {2, 2, 16, 16}; // Inner rectangle for the gold part
    SDL_FillRect(coin->img, &inner, SDL_MapRGB(coin->img->format, 255, 215, 0)); // Fill the inner part with gold
    coin->pos.w = 20; // Set the coin's width
    coin->pos.h = 20; // Set the coin's height
    coin->visible = 0; // Coin starts invisible
}

// Display a coin on the screen if it is visible
void displayCoin(Coin *coin, SDL_Surface *screen) {
    if (coin->visible) { // Only display if the coin is visible
        if (coin->img != NULL) {
            SDL_BlitSurface(coin->img, NULL, screen, &coin->pos); // Draw the coin on the screen
        } else {
            printf("Coin image is NULL, cannot render coin at position (%d, %d)\n", coin->pos.x, coin->pos.y);
        }
    }
}

// Check for collision between a coin and the player using a circular approximation
int collisionTriCoin(Coin *coin, SDL_Rect posPerso) {
    int estcoli;
    float R1, R2, X1, X2, D1, D2, Y1, Y2;
    X1 = posPerso.x + posPerso.w / 2; // Center x of the player
    Y1 = posPerso.y + posPerso.h / 2; // Center y of the player
    R1 = sqrt(pow(posPerso.w / 2, 2) + pow(posPerso.h / 2, 2)); // Player's radius
    if (posPerso.w < posPerso.h) {
        R1 = posPerso.w / 2;
    } else {
        R1 = posPerso.h / 2;
    }
    X2 = coin->pos.x + coin->pos.w / 2; // Center x of the coin
    Y2 = coin->pos.y + coin->pos.h / 2; // Center y of the coin
    R2 = sqrt(pow(coin->pos.w / 2, 2) + pow(coin->pos.h / 2, 2)); // Coin's radius
    if (coin->pos.w < coin->pos.h) {
        R2 = coin->pos.w / 2;
    } else {
        R2 = coin->pos.h / 2;
    }
    D1 = sqrt(pow(X2 - X1, 2) + pow(Y2 - Y1, 2)); // Distance between player and coin centers
    D2 = R1 + R2; // Sum of the radii
    if (D1 <= D2) { // If the distance is less than or equal to the sum of the radii, there is a collision
        estcoli = 1;
    } else {
        estcoli = 0;
    }
    return estcoli;
}

int main(int argc, char *argv[]) {
    int loop = 1; // Main game loop flag
    SDL_Surface *screen; // Main screen surface
    SDL_Event event; // Event handler for user input
    image IMAGE; // Background image
    Ennemi e, e1; // Two enemy bats
    Coin coin1, coin2; // Two collectible coins
    SDL_Surface *perso = IMG_Load("perso.png"); // Player character image
    SDL_Rect posPerso = {10, 450}; // Player's starting position
    int direction = -1; // Player movement direction (-1 = no movement, 0 = left, 1 = right, 2 = down, 3 = up)
    
    int health = 100; // Player's health (not used in current logic for player damage)
    const int max_health = 100; // Maximum health for the player
    Uint32 last_hit_time = 0; // Timestamp of the last hit (for cooldown)
    const Uint32 hit_cooldown = 500; // Cooldown between hits (500ms)

    int score = 0; // Player's score
    int both_coins_collected = 0; // Flag to track if both coins have been collected in the current cycle

    // Initialize SDL for video, audio, and timer
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return -1;
    }

    // Set up the screen with a resolution of 1060x594
    screen = SDL_SetVideoMode(1060, 594, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
    initialiser_imageBACK(&IMAGE); // Initialize the background
    initEnnemi(&e); // Initialize the first enemy
    initEnnemi1(&e1); // Initialize the second enemy
    initCoin(&coin1); // Initialize the first coin
    initCoin(&coin2); // Initialize the second coin

    Uint32 start; // Start time of each frame (for FPS control)
    const int FPS = 60; // Target frames per second
    while (loop) {
        start = SDL_GetTicks(); // Get the current time for FPS calculation
        Uint32 current_time = SDL_GetTicks(); // Current time for hit cooldown

        // Handle user input events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: // If the user closes the window, exit the game
                    loop = 0;
                    break;
                case SDL_KEYDOWN: // Handle key presses for movement
                    switch (event.key.keysym.sym) {
                        case SDLK_RIGHT: direction = 1; break;
                        case SDLK_LEFT:  direction = 0; break;
                        case SDLK_UP:    direction = 3; break;
                        case SDLK_DOWN:  direction = 2; break;
                    }
                    break;
                case SDL_KEYUP: // Stop movement when keys are released
                    switch (event.key.keysym.sym) {
                        case SDLK_RIGHT:
                        case SDLK_LEFT:
                        case SDLK_UP:
                        case SDLK_DOWN:
                            direction = -1;
                            break;
                    }
                    break;
            }
        }

        // Update the player's position based on the direction
        if (direction == 1) posPerso.x += 5; // Move right
        if (direction == 0) posPerso.x -= 5; // Move left
        if (direction == 3) posPerso.y -= 5; // Move up
        if (direction == 2) posPerso.y += 5; // Move down

        // Keep the player within the screen boundaries
        if (posPerso.x < 0) posPerso.x = 0;
        if (posPerso.x > 1060 - perso->w) posPerso.x = 1060 - perso->w;
        if (posPerso.y < 0) posPerso.y = 0;
        if (posPerso.y > 594 - perso->h) posPerso.y = 594 - perso->h;

        // Draw the background and the player
        afficher_imageBMP(screen, IMAGE);
        SDL_BlitSurface(perso, NULL, screen, &posPerso);
        
        // Handle the first enemy if it is alive
        if (e.alive) {
            afficherEnnemi(e, screen); // Display the enemy
            animerEnemi(&e); // Animate the enemy
            if (both_coins_collected) { // If both coins are collected, move horizontally
                moveHorizontal(&e);
            } else { // Otherwise, use the AI to chase the player
                moveIA(&e, posPerso);
            }
            draw_health_bar(screen, e.health, 50, e.pos_depart.x, e.pos_depart.y - 15, 40, 10); // Draw the enemy's health bar
        }

        // Handle the second enemy if it is alive
        if (e1.alive) {
            afficherEnnemi(e1, screen);
            animerEnemi(&e1);
            if (both_coins_collected) {
                moveHorizontal(&e1);
            } else {
                moveIA1(&e1, posPerso);
            }
            draw_health_bar(screen, e1.health, 50, e1.pos_depart.x, e1.pos_depart.y - 15, 40, 10);
        }

        // Check for collisions between the player and the enemies
        if (collisionTri(&e, posPerso) || collisuionBB(&e1, posPerso)) {
            if (current_time - last_hit_time >= hit_cooldown) { // Only apply damage if the cooldown has passed
                int e_was_alive = e.alive; // Track if the enemy was alive before taking damage
                e.health -= 10; // Reduce the first enemy's health
                if (e.health <= 0 && e_was_alive) { // If the enemy is defeated
                    e.alive = 0; // Mark the enemy as defeated
                    score += 100; // Add points to the score
                    printf("Bat defeated! Score: %d\n", score);
                    coin1.pos.x = e.pos_depart.x; // Drop a coin at the enemy's position
                    coin1.pos.y = e.pos_depart.y;
                    coin1.visible = 1; // Make the coin visible
                    printf("Coin 1 dropped at (%d, %d)\n", coin1.pos.x, coin1.pos.y);
                }

                int e1_was_alive = e1.alive;
                e1.health -= 10; // Reduce the second enemy's health
                if (e1.health <= 0 && e1_was_alive) {
                    e1.alive = 0;
                    score += 100;
                    printf("Bat defeated! Score: %d\n", score);
                    coin2.pos.x = e1.pos_depart.x;
                    coin2.pos.y = e1.pos_depart.y;
                    coin2.visible = 1;
                    printf("Coin 2 dropped at (%d, %d)\n", coin2.pos.x, coin2.pos.y);
                }

                last_hit_time = current_time; // Update the last hit time
            }
        }

        // Check if the player collects the first coin
        if (coin1.visible && collisionTriCoin(&coin1, posPerso)) {
            coin1.visible = 0; // Hide the coin
            score += 50; // Add points to the score
            printf("Coin 1 collected! Score: %d\n", score);
        }

        // Check if the player collects the second coin
        if (coin2.visible && collisionTriCoin(&coin2, posPerso)) {
            coin2.visible = 0;
            score += 50;
            printf("Coin 2 collected! Score: %d\n", score);
        }

        // If both coins are collected and both enemies are defeated, respawn the enemies
        if (!coin1.visible && !coin2.visible && !e.alive && !e1.alive && !both_coins_collected) {
            // Respawn the first enemy at the bottom-left corner
            e.pos_depart.x = 12;
            e.pos_depart.y = 594 - e.pos_sprites.h;
            e.health = 50;
            e.alive = 1;
            e.state = WAITING;
            printf("Bat e respawned at bottom-left corner (%d, %d)\n", e.pos_depart.x, e.pos_depart.y);

            // Respawn the second enemy at the bottom-right corner
            e1.pos_depart.x = 1060 - e1.pos_sprites.w;
            e1.pos_depart.y = 594 - e1.pos_sprites.h;
            e1.health = 50;
            e1.alive = 1;
            e1.state = WAITING;
            printf("Bat e1 respawned at bottom-right corner (%d, %d)\n", e1.pos_depart.x, e1.pos_depart.y);

            both_coins_collected = 1; // Mark that both coins have been collected
        }

        // Reset the flag when both enemies are defeated again, allowing a new cycle
        if (!e.alive && !e1.alive && both_coins_collected) {
            both_coins_collected = 0;
        }

        // Display the coins on the screen
        displayCoin(&coin1, screen);
        displayCoin(&coin2, screen);

        // Draw the player's health bar (though player health isn't modified in this code)
        draw_health_bar(screen, health, max_health, 840, 20, 200, 20);

        SDL_Flip(screen); // Update the screen to show the new frame
        // Control the frame rate to maintain 60 FPS
        if (1000/FPS > SDL_GetTicks() - start)
            SDL_Delay(1000/FPS - (SDL_GetTicks() - start));
    }

    // Clean up resources before exiting
    SDL_FreeSurface(IMAGE.img);
    SDL_FreeSurface(perso);
    SDL_FreeSurface(coin1.img);
    SDL_FreeSurface(coin2.img);
    SDL_Quit();
    return 0;
}
