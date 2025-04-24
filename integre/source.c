#include "header.h"
#include <stdlib.h>
#include <string.h>

int getHoveredButtonAt(ButtonImg buttons[], int numButtons, int mouseX, int mouseY) {
    for (int i = 0; i < numButtons; i++) {
        if (mouseX >= buttons[i].rect.x &&
            mouseX <= (buttons[i].rect.x + buttons[i].rect.w) &&
            mouseY >= buttons[i].rect.y &&
            mouseY <= (buttons[i].rect.y + buttons[i].rect.h)) {
            return i;
        }
    }
    return NO_HOVER;
}

void initialiser_bouton(ButtonImg *btn, const char *chemin, int x, int y, const char* text, TTF_Font* font) {
    btn->image = IMG_Load(chemin);
    if (btn->image == NULL) {
        fprintf(stderr, "Erreur lors du chargement du bouton : %s\n", SDL_GetError());
        return;
    }
    btn->rect.x = x;
    btn->rect.y = y;
    btn->rect.w = btn->image->w;
    btn->rect.h = btn->image->h;
    
    if (text && font) {
        SDL_Color textColor = {255, 255, 255};
        btn->textSurface = TTF_RenderText_Blended(font, text, textColor);
        if (btn->textSurface) {
            btn->textRect.x = btn->rect.x + (btn->rect.w - btn->textSurface->w) / 2;
            btn->textRect.y = btn->rect.y + (btn->rect.h - btn->textSurface->h) / 2;
        }
    }
}

void updateAnswerButtons(ButtonImg normalButtons[], ButtonImg hoveredButtons[], 
                        const char answers[][MAX_ANSWER_LENGTH], TTF_Font* font) {
    for (int i = 0; i < 3; i++) {
        int btnIndex = i + 2;
        
        if (normalButtons[btnIndex].textSurface) {
            SDL_FreeSurface(normalButtons[btnIndex].textSurface);
        }
        if (hoveredButtons[btnIndex].textSurface) {
            SDL_FreeSurface(hoveredButtons[btnIndex].textSurface);
        }
        
        SDL_Color textColor = {255, 255, 255};
        normalButtons[btnIndex].textSurface = TTF_RenderText_Blended(font, answers[i], textColor);
        hoveredButtons[btnIndex].textSurface = TTF_RenderText_Blended(font, answers[i], textColor);
        
        if (normalButtons[btnIndex].textSurface) {
            normalButtons[btnIndex].textRect.x = normalButtons[btnIndex].rect.x + 
                (normalButtons[btnIndex].rect.w - normalButtons[btnIndex].textSurface->w) / 2;
            normalButtons[btnIndex].textRect.y = normalButtons[btnIndex].rect.y + 
                (normalButtons[btnIndex].rect.h - normalButtons[btnIndex].textSurface->h) / 2;
        }
        
        if (hoveredButtons[btnIndex].textSurface) {
            hoveredButtons[btnIndex].textRect.x = hoveredButtons[btnIndex].rect.x + 
                (hoveredButtons[btnIndex].rect.w - hoveredButtons[btnIndex].textSurface->w) / 2;
            hoveredButtons[btnIndex].textRect.y = hoveredButtons[btnIndex].rect.y + 
                (hoveredButtons[btnIndex].rect.h - hoveredButtons[btnIndex].textSurface->h) / 2;
        }
    }
}

int loadQuestions(Question questions[], const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;

    for (int i = 0; i < MAX_QUESTIONS; i++) {
        if (!fgets(questions[i].question, MAX_QUESTION_LENGTH, file)) break;
        questions[i].question[strcspn(questions[i].question, "\n")] = 0;
        
        char tempAnswers[3][MAX_ANSWER_LENGTH];
        for (int j = 0; j < 3; j++) {
            if (!fgets(tempAnswers[j], MAX_ANSWER_LENGTH, file)) break;
            tempAnswers[j][strcspn(tempAnswers[j], "\n")] = 0;
        }
        
        // Shuffle answers
        int indices[3] = {0, 1, 2};
        for (int j = 2; j > 0; j--) {
            int k = rand() % (j + 1);
            int temp = indices[j];
            indices[j] = indices[k];
            indices[k] = temp;
        }
        
        // Assign shuffled answers and track correct answer
        int correctIndex = 0; // Original correct answer is always first
        for (int j = 0; j < 3; j++) {
            strcpy(questions[i].answers[j], tempAnswers[indices[j]]);
            if (indices[j] == 0) {
                questions[i].correctAnswer = j;
            }
        }
        
        questions[i].used = 0;
    }
    
    fclose(file);
    return 1;
}

Question* getRandomQuestion(Question questions[]) {
    int available = 0;
    int indices[MAX_QUESTIONS];
    
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        if (!questions[i].used) indices[available++] = i;
    }
    
    if (available == 0) return NULL;
    
    int randomIndex = indices[rand() % available];
    questions[randomIndex].used = 1;
    return &questions[randomIndex];
}

int checkAnswer(Question* q, int answerIndex, GameState* state) {
    if (answerIndex == q->correctAnswer) {
        state->score += 10 * state->level;
        return 1;
    } else {
        state->lives--;
        return 0;
    }
}

void initTimerBar(TimerBar* timer, const char* imagePath, int x, int y, SDL_Surface* screen) {
    timer->fullTimer = IMG_Load(imagePath);
    if (!timer->fullTimer) return;
    
    timer->position.x = x;
    timer->position.y = y;
    timer->position.w = timer->fullTimer->w;
    timer->position.h = timer->fullTimer->h;
    timer->maxWidth = timer->fullTimer->w;
    
    timer->currentTimer = SDL_CreateRGBSurface(SDL_SWSURFACE, 
                                             timer->maxWidth, 
                                             timer->fullTimer->h, 
                                             screen->format->BitsPerPixel,
                                             0, 0, 0, 0);
    
    timer->cropRect.x = 0;
    timer->cropRect.y = 0;
    timer->cropRect.w = timer->maxWidth;
    timer->cropRect.h = timer->fullTimer->h;
}

void updateTimerBar(TimerBar* timer, float timeRatio, SDL_Surface* screen) {
    int newWidth = (int)(timer->maxWidth * timeRatio);
    timer->cropRect.w = newWidth > 0 ? newWidth : 0;
    SDL_FillRect(timer->currentTimer, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(timer->fullTimer, &timer->cropRect, timer->currentTimer, NULL);
}

void renderTimerBar(SDL_Surface* screen, TimerBar* timer) {
    SDL_BlitSurface(timer->currentTimer, NULL, screen, &timer->position);
}

void freeTimerBar(TimerBar* timer) {
    if (timer->fullTimer) SDL_FreeSurface(timer->fullTimer);
    if (timer->currentTimer) SDL_FreeSurface(timer->currentTimer);
}

void updateGameState(GameState* state) {
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedSeconds = (currentTime - state->startTime) / 1000;
    state->timeLeft = TOTAL_QUIZ_TIME - elapsedSeconds;
    
    if (state->timeLeft <= 0) {
        state->lives--;
        state->startTime = SDL_GetTicks();
    }
}
