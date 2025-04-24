#ifndef HEADER_H
#define HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>

#define NUM_BUTTONS 5
#define NO_HOVER 999
#define MAX_QUESTIONS 10
#define MAX_QUESTION_LENGTH 256
#define MAX_ANSWER_LENGTH 100
#define TOTAL_QUIZ_TIME 60

typedef struct {
    SDL_Rect rect;
    SDL_Surface *image;
    SDL_Surface *textSurface;
    SDL_Rect textRect;
} ButtonImg;

typedef struct {
    char question[MAX_QUESTION_LENGTH];
    char answers[3][MAX_ANSWER_LENGTH];
    int correctAnswer;
    int used;
} Question;

typedef struct {
    int score;
    int lives;
    int level;
    int timeLeft;
    Uint32 startTime;
} GameState;

typedef struct {
    SDL_Surface* fullTimer;
    SDL_Surface* currentTimer;
    SDL_Rect position;
    SDL_Rect cropRect;
    int maxWidth;
} TimerBar;

int getHoveredButtonAt(ButtonImg buttons[], int numButtons, int mouseX, int mouseY);
void initialiser_bouton(ButtonImg *btn, const char *chemin, int x, int y, const char* text, TTF_Font* font);
void updateAnswerButtons(ButtonImg normalButtons[], ButtonImg hoveredButtons[], const char answers[][MAX_ANSWER_LENGTH], TTF_Font* font);
int loadQuestions(Question questions[], const char* filename);
Question* getRandomQuestion(Question questions[]);
int checkAnswer(Question* q, int answerIndex, GameState* state);
void initTimerBar(TimerBar* timer, const char* imagePath, int x, int y, SDL_Surface* screen);
void updateTimerBar(TimerBar* timer, float timeRatio, SDL_Surface* screen);
void renderTimerBar(SDL_Surface* screen, TimerBar* timer);
void freeTimerBar(TimerBar* timer);
void updateGameState(GameState* state);

#endif // HEADER_H
