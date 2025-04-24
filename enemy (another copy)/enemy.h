#ifndef ENEMY_H_INCLUDED
#define ENEMY_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
/*---------------------------------------------------*/

// Define possible states for the enemy (bat) behavior
enum STATE
{
  WAITING,   // Enemy is idle, moving in a default pattern
  FOLLOWING, // Enemy is chasing the player
  ATTACKING  // Enemy is close enough to attack the player
};
typedef enum STATE STATE;

// Structure to represent an enemy (bat)
typedef struct
{
  SDL_Rect pos_depart;      // Starting position of the enemy on the screen
  SDL_Rect pos_actuelle;    // Current position of the enemy (used for movement tracking)
  int direction;            // Direction of movement (0 = up/left, 1 = down/right)
  float vitesse;            // Speed of the enemy (currently unused in movement)
  SDL_Surface *spritesheet; // Image containing all animation frames for the enemy
  SDL_Rect pos_sprites;     // Portion of the spritesheet to display (for animation)
  int frame;                // Current frame of animation
  int frameCount;           // Total number of frames in the spritesheet
  int frameWidth;           // Width of each frame in the spritesheet
  int frameHeight;          // Height of each frame in the spritesheet
  int alive;                // Flag to check if the enemy is alive (1 = alive, 0 = defeated)
  STATE state;              // Current state of the enemy (WAITING, FOLLOWING, ATTACKING)
  int health;               // Health points of the enemy (decreases when hit)
} Ennemi;

// Structure to represent a background image
typedef struct
{
  char *url;               // File path of the background image
  SDL_Rect pos_img_affiche;// Portion of the image to display (for scrolling or cropping)
  SDL_Rect pos_img_ecran;  // Position on the screen where the image is displayed
  SDL_Surface *img;        // The actual image data loaded into memory
} image;

// Structure to represent a collectible coin
typedef struct
{
  SDL_Rect pos;            // Position of the coin on the screen
  SDL_Surface *img;        // Image of the coin
  int visible;             // Flag to check if the coin is visible (1 = visible, 0 = collected/hidden)
} Coin;

// Function declarations for background handling
void initialiser_imageBACK(image *image); // Loads and sets up the background image
void afficher_imageBMP(SDL_Surface *screen, image image); // Displays the background image on the screen

// Function declarations for enemy handling
void initEnnemi(Ennemi *e); // Initializes the first enemy (bat) with starting values
void initEnnemi1(Ennemi *e); // Initializes the second enemy (bat) with slightly different starting values
void afficherEnnemi(Ennemi e, SDL_Surface *screen); // Displays the enemy on the screen with visual effects
void animerEnemi(Ennemi *e); // Updates the enemy's animation frame
void move(Ennemi *e); // Moves the first enemy vertically at a specific speed
void move1(Ennemi *e); // Moves the second enemy vertically at a different speed
void moveHorizontal(Ennemi *e); // Moves the enemy horizontally (used after coins are collected)
void updateEnnemiState(Ennemi *E, int distx, int disty); // Updates the enemy's state based on distance to the player
void moveEnnemi(Ennemi *e, SDL_Rect posperso); // Makes the first enemy follow the player
void moveEnnemi1(Ennemi *e, SDL_Rect posperso); // Makes the second enemy follow the player (faster speed)
void moveIA(Ennemi *E, SDL_Rect posperso); // Controls the first enemy's AI (state-based movement)
void moveIA1(Ennemi *E, SDL_Rect posperso); // Controls the second enemy's AI (state-based movement)
int collisuionBB(Ennemi *E, SDL_Rect posPerso); // Checks for collision between enemy and player using bounding box method
int collisionTri(Ennemi *e, SDL_Rect posPerso); // Checks for collision between enemy and player using circular approximation

#endif
