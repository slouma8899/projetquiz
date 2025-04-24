#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include "enemy.h"

// Initialize the background image with a file and set its display properties
void initialiser_imageBACK(image *image)
{
    image->url = "background.png"; // Set the file path for the background image
    image->img = IMG_Load(image->url); // Load the image using SDL_image
    if (image->img == NULL) // Check if the image failed to load
    {
        printf("unable to load background image %s \n", SDL_GetError());
    }
    image->pos_img_ecran.x = 0; // Set the screen position to top-left corner
    image->pos_img_ecran.y = 0;

    image->pos_img_affiche.x = 0; // Set the portion of the image to display (no cropping here)
    image->pos_img_affiche.y = 0;
    image->pos_img_affiche.h = 1060; // Height of the display area (matches screen width)
    image->pos_img_affiche.w = 594;  // Width of the display area (matches screen height)
}

// Display the background image on the screen
void afficher_imageBMP(SDL_Surface *screen, image image)
{
    SDL_BlitSurface(image.img, NULL, screen, &image.pos_img_ecran); // Copy the background image to the screen
}

// Initialize the first enemy (bat) with starting values
void initEnnemi(Ennemi *e)
{
    e->pos_depart.x = 260; // Starting x position
    e->pos_depart.y = 100; // Starting y position
    e->pos_actuelle = e->pos_depart; // Current position starts at the starting position
    e->direction = 0; // Initial direction (0 = up)
    e->vitesse = 0;   // Speed (not used in movement logic)
    e->alive = 1;     // Enemy starts alive
    e->health = 50;   // Enemy starts with 50 health points

    e->spritesheet = IMG_Load("bat.png"); // Load the sprite sheet for the bat
    if (e->spritesheet == NULL) // Check if the sprite sheet failed to load
    {
        printf("Erreur lors du chargement de la spritesheet de l'ennemi : %s\n", SDL_GetError());
    }

    e->frame = 0;           // Start at the first frame of animation
    e->frameCount = 4;      // Total of 4 frames in the sprite sheet
    e->frameWidth = 35;     // Width of each frame
    e->frameHeight = 55;    // Height of each frame

    e->pos_sprites.x = 0;   // Start at the first frame (x position in sprite sheet)
    e->pos_sprites.y = 0;   // Y position in sprite sheet (single row assumed)
    e->pos_sprites.w = e->frameWidth; // Width of the frame to display
    e->pos_sprites.h = e->frameHeight;// Height of the frame to display
    e->state = WAITING;     // Start in WAITING state (default movement)
}

// Initialize the second enemy (bat) with slightly different starting values
void initEnnemi1(Ennemi *e)
{
    e->pos_depart.x = 300; // Different starting x position
    e->pos_depart.y = 300; // Different starting y position
    e->pos_actuelle = e->pos_depart;
    e->direction = 0;
    e->vitesse = 0;
    e->alive = 1;
    e->health = 50;

    e->spritesheet = IMG_Load("bat.png");
    if (e->spritesheet == NULL)
    {
        printf("Erreur lors du chargement de la spritesheet de l'ennemi : %s\n", SDL_GetError());
    }

    e->frame = 0;
    e->frameCount = 4;
    e->frameWidth = 35;
    e->frameHeight = 55;

    e->pos_sprites.x = 0;
    e->pos_sprites.y = 0;
    e->pos_sprites.w = e->frameWidth;
    e->pos_sprites.h = e->frameHeight;
    e->state = WAITING;
}

// Display the enemy on the screen with visual effects based on state and health
void afficherEnnemi(Ennemi e, SDL_Surface *screen)
{
    if (e.health < 50) { // If health is below 50, apply a red tint to indicate damage
        SDL_SetAlpha(e.spritesheet, SDL_SRCALPHA | SDL_RLEACCEL, 128); // 50% transparency
        SDL_SetColorKey(e.spritesheet, SDL_SRCCOLORKEY, SDL_MapRGB(e.spritesheet->format, 255, 0, 0));
    }
    else if (e.state == ATTACKING) { // If attacking, apply a yellow tint to indicate aggression
        SDL_SetAlpha(e.spritesheet, SDL_SRCALPHA | SDL_RLEACCEL, 128);
        SDL_SetColorKey(e.spritesheet, SDL_SRCCOLORKEY, SDL_MapRGB(e.spritesheet->format, 255, 255, 0));
    }
    SDL_BlitSurface(e.spritesheet, &e.pos_sprites, screen, &e.pos_depart); // Draw the enemy on the screen
    if (e.health < 50 || e.state == ATTACKING) { // Reset visual effects after drawing
        SDL_SetAlpha(e.spritesheet, SDL_SRCALPHA | SDL_RLEACCEL, 255); // Full opacity
        SDL_SetColorKey(e.spritesheet, 0, 0); // Remove color key
    }
}

// Animate the enemy by cycling through its sprite frames
void animerEnemi(Ennemi *e)
{
    e->frame = (e->frame + 1) % e->frameCount; // Move to the next frame, looping back to 0 after the last frame
    e->pos_sprites.x = e->frame * e->frameWidth; // Update the x position in the sprite sheet to show the current frame
}

// Move the first enemy vertically with a specific speed
void move1(Ennemi *e)
{
    if (e->pos_depart.y < 12) // If the enemy is near the top of the screen, move down
        e->direction = 1;
    else if (e->pos_depart.y > 400) // If the enemy is near the bottom, move up
        e->direction = 0;

    if (e->direction == 1) // Move down by 10 pixels
        e->pos_depart.y += 10;
    if (e->direction == 0) // Move up by 10 pixels
        e->pos_depart.y -= 10;
}

// Move the first enemy vertically at a slower speed (for the first enemy)
void move(Ennemi *e)
{
    if (e->pos_depart.y < 12)
        e->direction = 1;
    else if (e->pos_depart.y > 400)
        e->direction = 0;

    if (e->direction == 1) // Move down by 7 pixels (slower than move1)
        e->pos_depart.y += 7;
    if (e->direction == 0) // Move up by 7 pixels
        e->pos_depart.y -= 7;
}

// Move the enemy horizontally (used after both coins are collected)
void moveHorizontal(Ennemi *e)
{
    if (e->pos_depart.x < 12) // If near the left edge, move right
        e->direction = 1;
    else if (e->pos_depart.x > 1060 - e->pos_sprites.w) // If near the right edge, move left
        e->direction = 0;

    if (e->direction == 1) // Move right by 5 pixels
        e->pos_depart.x += 5;
    if (e->direction == 0) // Move left by 5 pixels
        e->pos_depart.x -= 5;
}

// Make the first enemy follow the player
void moveEnnemi(Ennemi *e, SDL_Rect posperso)
{
    if (e->pos_depart.x > posperso.x) // If enemy is to the right of the player, move left
    {
        e->pos_depart.x -= 3;

        if (e->pos_depart.y > posperso.y) // If enemy is below the player, move up
        {
            e->pos_depart.y -= 3;
        }
        if (e->pos_depart.y < posperso.y) // If enemy is above the player, move down
        {
            e->pos_depart.y = e->pos_depart.y + 3;
        }
    }
    if (e->pos_depart.x < posperso.x) // If enemy is to the left of the player, move right
    {
        e->pos_depart.x = e->pos_depart.x + 3;

        if (e->pos_depart.y > posperso.y)
        {
            e->pos_depart.y -= 3;
        }
        if (e->pos_depart.y < posperso.y)
        {
            e->pos_depart.y = e->pos_depart.y + 3;
        }
    }
}

// Make the second enemy follow the player (faster speed)
void moveEnnemi1(Ennemi *e, SDL_Rect posperso)
{
    if (e->pos_depart.x > posperso.x) // Same logic as moveEnnemi but with a faster speed (5 pixels)
    {
        e->pos_depart.x -= 5;

        if (e->pos_depart.y > posperso.y)
        {
            e->pos_depart.y -= 5;
        }
        if (e->pos_depart.y < posperso.y)
        {
            e->pos_depart.y = e->pos_depart.y + 5;
        }
    }
    if (e->pos_depart.x < posperso.x)
    {
        e->pos_depart.x = e->pos_depart.x + 5;

        if (e->pos_depart.y > posperso.y)
        {
            e->pos_depart.y -= 5;
        }
        if (e->pos_depart.y < posperso.y)
        {
            e->pos_depart.y = e->pos_depart.y + 5;
        }
    }
}

// Update the enemy's state based on its distance from the player
void updateEnnemiState(Ennemi *E, int distx, int disty)
{
    if (distx > 100 && disty > 100) { // If far from the player, enter WAITING state
        E->state = WAITING;
    }
    else if (distx <= 50 && disty <= 50) { // If very close to the player, enter ATTACKING state
        E->state = ATTACKING;
    }
    else { // If at a medium distance, enter FOLLOWING state
        E->state = FOLLOWING;
    }
}

// Control the first enemy's AI based on its state
void moveIA(Ennemi *E, SDL_Rect posperso)
{
    int dx, dy;
    switch (E->state)
    {
    case WAITING: // In WAITING state, move vertically using the default movement
        move(E);
        break;
    case FOLLOWING: // In FOLLOWING state, chase the player
        moveEnnemi(E, posperso);
        break;
    case ATTACKING: // In ATTACKING state, stop moving to simulate attacking
        E->vitesse = 0;
        break;
    }
    
    dx = abs(E->pos_depart.x - posperso.x); // Calculate x distance to player
    dy = abs(E->pos_depart.y - posperso.y); // Calculate y distance to player
    updateEnnemiState(E, dx, dy); // Update the state based on the new distance
}

// Control the second enemy's AI (similar to moveIA but uses move1 for default movement)
void moveIA1(Ennemi *E, SDL_Rect posperso)
{
    int dx, dy;
    switch (E->state)
    {
    case WAITING:
        move1(E);
        break;
    case FOLLOWING:
        moveEnnemi1(E, posperso);
        break;
    case ATTACKING:
        E->vitesse = 0;
        break;
    }
    
    dx = abs(E->pos_depart.x - posperso.x);
    dy = abs(E->pos_depart.y - posperso.y);
    updateEnnemiState(E, dx, dy);
}

// Check for collision between the enemy and the player using a bounding box method
int collisuionBB(Ennemi *E, SDL_Rect posPerso)
{
    int i = 1; // Assume collision by default
    if (E->pos_depart.y > posPerso.y + posPerso.h) // If enemy is below the player, no collision
    {
        i = 0;
        return i;
    }
    if (E->pos_depart.x > posPerso.x + posPerso.w) // If enemy is to the right of the player, no collision
    {
        i = 0;
        return i;
    }
    if (E->pos_depart.y + E->pos_depart.h < posPerso.y) // If enemy is above the player, no collision
    {
        i = 0;
        return i;
    }
    if (E->pos_depart.x + E->pos_depart.w < posPerso.x) // If enemy is to the left of the player, no collision
    {
        i = 0;
        return i;
    }
    return i; // If none of the above conditions are met, there is a collision
}

// Check for collision between the enemy and the player using a circular approximation
int collisionTri(Ennemi *e, SDL_Rect posPerso) {
    int estcoli;
    float R1, R2, X1, X2, D1, D2, Y1, Y2;
    X1 = posPerso.x + posPerso.w / 2; // Center x of the player
    Y1 = posPerso.y + posPerso.h / 2; // Center y of the player
    R1 = sqrt(pow(posPerso.w / 2, 2) + pow(posPerso.h / 2, 2)); // Radius of the player's bounding circle
    if (posPerso.w < posPerso.h) // Use the smaller dimension as the radius for better accuracy
    {
        R1 = posPerso.w / 2;
    }
    else
    {
        R1 = posPerso.h / 2;
    }
    X2 = e->pos_depart.x + e->pos_depart.w / 2; // Center x of the enemy
    Y2 = e->pos_depart.y + e->pos_depart.h / 2; // Center y of the enemy
    R2 = sqrt(pow(e->pos_depart.w / 2, 2) + pow(e->pos_depart.h / 2, 2)); // Radius of the enemy's bounding circle
    if (e->pos_depart.w < e->pos_depart.h)
    {
        R2 = e->pos_depart.w / 2;
    }
    else
    {
        R2 = e->pos_depart.h / 2;
    }
    D1 = sqrt(pow(X2 - X1, 2) + pow(Y2 - Y1, 2)); // Distance between the centers of the player and enemy
    D2 = R1 + R2; // Sum of the radii
    if (D1 <= D2) // If the distance is less than or equal to the sum of the radii, there is a collision
    {
        estcoli = 1;
    }
    else
    {
        estcoli = 0;
    }
    return estcoli;
}
