#include "header.h"
#include "enigme2.h"
#include <stdlib.h>
#include <time.h>

// Global sound and font definitions for puzzle game
Mix_Chunk *flipSound = NULL;
Mix_Chunk *matchSound = NULL;
Mix_Chunk *wrongSound = NULL;
Mix_Chunk *winSound = NULL;
Mix_Chunk *loseSound = NULL;
TTF_Font *gFont = NULL;

void runPuzzleGame(SDL_Surface *screen) {
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return;
    }
    gFont = TTF_OpenFont("arial.ttf", 48);
    if (!gFont) {
        printf("Error loading font: %s\n", TTF_GetError());
        TTF_Quit();
        return;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        printf("Mix_OpenAudio: %s\n", Mix_GetError());
        TTF_CloseFont(gFont);
        TTF_Quit();
        return;
    }

    // Load puzzle game sounds
    flipSound = Mix_LoadWAV("sounds/flip.wav");
    if (!flipSound)
        printf("Error loading flip.wav: %s\n", Mix_GetError());
    matchSound = Mix_LoadWAV("sounds/match.wav");
    if (!matchSound)
        printf("Error loading match.wav: %s\n", Mix_GetError());
    wrongSound = Mix_LoadWAV("sounds/wrong.wav");
    if (!wrongSound)
        printf("Error loading wrong.wav: %s\n", Mix_GetError());
    winSound = Mix_LoadWAV("sounds/win.wav");
    if (!winSound)
        printf("Error loading win.wav: %s\n", Mix_GetError());
    loseSound = Mix_LoadWAV("sounds/lose.wav");
    if (!loseSound)
        printf("Error loading lose.wav: %s\n", Mix_GetError());

    // Set difficulty (default to 3 for Extreme, as in original)
    int difficulty = 3;
    int exit_requested = 0;
    int restart_requested = 0;

    while (!exit_requested) {
        int grid_size;
        if (difficulty == 1)
            grid_size = 2;
        else if (difficulty == 2)
            grid_size = 4;
        else // difficulty == 3
            grid_size = 4;

        MemoryGame game;
        initialiser_enigme(&game, ".", grid_size, difficulty); // Use current directory for assets

        int running = 1;
        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_q) {
                    running = 0;
                    exit_requested = 1;
                }
                if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_r) {
                    running = 0;
                    restart_requested = 1;
                }
                if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_d) {
                    difficulty = 1;
                    printf("Difficulty reset to level 1 (2x2)\n");
                }
                if (ev.type == SDL_QUIT) {
                    running = 0;
                    exit_requested = 1;
                }
                Memory_HandleEvent(&game, &ev);
            }
            Memory_Update(&game);
            Memory_Render(&game, screen);
            SDL_Flip(screen);
            SDL_Delay(16);
        }
        Memory_Cleanup(&game);
        if (restart_requested)
            restart_requested = 0;
    }

    // Cleanup puzzle game resources
    Mix_FreeChunk(flipSound);
    Mix_FreeChunk(matchSound);
    Mix_FreeChunk(wrongSound);
    Mix_FreeChunk(winSound);
    Mix_FreeChunk(loseSound);
    Mix_CloseAudio();
    TTF_CloseFont(gFont);
    TTF_Quit();
}

int main(int argc, char *argv[]) {
    // Initialize random seed
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // Set window size to accommodate puzzle game (800x600 as in enigme2.c)
    SDL_Surface *screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE | SDL_SRCALPHA);
    if (!screen) {
        printf("SDL_SetVideoMode error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_WM_SetCaption("Menu Enigme", NULL);

    if (TTF_Init() == -1) {
        printf("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("fonti.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface *background = IMG_Load("bg.jpeg");
    SDL_Surface *winScreen = IMG_Load("win.png");
    SDL_Surface *loseScreen = IMG_Load("lose.png");
    if (!background || !winScreen || !loseScreen) {
        printf("Error loading images: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Scale images to fit window
    SDL_Surface *scaledWin = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 600, 32, 0, 0, 0, 0);
    SDL_Surface *scaledLose = SDL_CreateRGBSurface(SDL_SWSURFACE, 800, 600, 32, 0, 0, 0, 0);
    SDL_SoftStretch(winScreen, NULL, scaledWin, NULL);
    SDL_SoftStretch(loseScreen, NULL, scaledLose, NULL);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio error: %s\n", Mix_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Mix_Chunk *hoverSound = Mix_LoadWAV("sfx.wav");
    Mix_Music *suspenseMusic = Mix_LoadMUS("bgmusic.mp3");
    Mix_Chunk *winSound = Mix_LoadWAV("win.wav");
    Mix_Chunk *loseSound = Mix_LoadWAV("lose.wav");

    Question questions[MAX_QUESTIONS];
    GameState gameState = {0, 3, 1, TOTAL_QUIZ_TIME, 0};
    Question* currentQuestion = NULL;
    TimerBar gameTimer;

    if (!loadQuestions(questions, "sciencefiction_quiz.txt")) {
        printf("Failed to load questions\n");
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    initTimerBar(&gameTimer, "timer_bar.png", 50, 50, screen);

    ButtonImg normalButtons[NUM_BUTTONS];
    ButtonImg hoveredButtons[NUM_BUTTONS];

    // Initialize buttons (adjusted positions for larger window)
    initialiser_bouton(&normalButtons[0], "quiz.png", 200, 200, NULL, NULL);
    initialiser_bouton(&normalButtons[1], "puzzle.png", 450, 200, NULL, NULL);
    initialiser_bouton(&normalButtons[2], "reponse_a.png", 100, 150, "Answer 1", font);
    initialiser_bouton(&normalButtons[3], "reponse_b.png", 300, 150, "Answer 2", font);
    initialiser_bouton(&normalButtons[4], "reponse_c.png", 500, 150, "Answer 3", font);

    initialiser_bouton(&hoveredButtons[0], "quizl.png", 200, 200, NULL, NULL);
    initialiser_bouton(&hoveredButtons[1], "puzzlel.png", 450, 200, NULL, NULL);
    initialiser_bouton(&hoveredButtons[2], "reponse_al.png", 100, 150, "Answer 1", font);
    initialiser_bouton(&hoveredButtons[3], "reponse_bl.png", 300, 150, "Answer 2", font);
    initialiser_bouton(&hoveredButtons[4], "reponse_cl.png", 500, 150, "Answer 3", font);

    int running = 1;
    int inQuiz = 0;
    int inPuzzle = 0;
    int gameEnded = 0;
    int questionsAnswered = 0;
    float animationAlpha = 0.0f;
    SDL_Event event;
    int currentHovered = NO_HOVER;
    int mouseX = 0, mouseY = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_MOUSEMOTION) {
                mouseX = event.motion.x;
                mouseY = event.motion.y;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && !gameEnded && !inPuzzle) {
                if (inQuiz == 0) {
                    int clickedButton = getHoveredButtonAt(normalButtons, NUM_BUTTONS, mouseX, mouseY);
                    if (clickedButton == 0) {
                        inQuiz = 1;
                        questionsAnswered = 0;
                        currentQuestion = getRandomQuestion(questions);
                        if (currentQuestion) {
                            updateAnswerButtons(normalButtons, hoveredButtons, currentQuestion->answers, font);
                            gameState.startTime = SDL_GetTicks();
                        }
                        if (suspenseMusic) Mix_PlayMusic(suspenseMusic, -1);
                    } else if (clickedButton == 1) {
                        inPuzzle = 1;
                        runPuzzleGame(screen);
                        inPuzzle = 0;
                        // Reinitialize SDL_ttf and SDL_mixer for quiz game
                        if (TTF_Init() == -1) {
                            printf("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
                            running = 0;
                        }
                        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                            printf("Mix_OpenAudio error: %s\n", Mix_GetError());
                            running = 0;
                        }
                    }
                } else {
                    int answerSelected = getHoveredButtonAt(normalButtons, NUM_BUTTONS, mouseX, mouseY);
                    if (answerSelected >= 2 && answerSelected <= 4 && currentQuestion) {
                        int answerIndex = answerSelected - 2;
                        checkAnswer(currentQuestion, answerIndex, &gameState);
                        questionsAnswered++;
                        
                        if (questionsAnswered >= MAX_QUESTIONS) {
                            gameEnded = 1;
                            inQuiz = 0;
                            Mix_HaltMusic();
                            if (winSound) Mix_PlayChannel(-1, winSound, 0);
                        } else {
                            currentQuestion = getRandomQuestion(questions);
                            if (currentQuestion) {
                                updateAnswerButtons(normalButtons, hoveredButtons, currentQuestion->answers, font);
                                gameState.startTime = SDL_GetTicks();
                            }
                        }
                    }
                }
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else if (event.key.keysym.sym == SDLK_r && gameEnded) {
                    // Reset game
                    gameEnded = 0;
                    inQuiz = 0;
                    gameState = (GameState){0, 3, 1, TOTAL_QUIZ_TIME, 0};
                    questionsAnswered = 0;
                    animationAlpha = 0.0f;
                    for (int i = 0; i < MAX_QUESTIONS; i++) questions[i].used = 0;
                    Mix_HaltMusic();
                }
            }
        }

        int hoveredIndex = getHoveredButtonAt(normalButtons, NUM_BUTTONS, mouseX, mouseY);
        if (hoveredIndex != currentHovered && !gameEnded && !inPuzzle) {
            if (hoveredIndex != NO_HOVER && hoverSound) {
                if ((inQuiz && hoveredIndex >= 2) || (!inQuiz && hoveredIndex < 2)) {
                    Mix_PlayChannel(-1, hoverSound, 0);
                }
            }
            currentHovered = hoveredIndex;
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

        if (gameEnded) {
            // Fade animation
            animationAlpha += 0.02f;
            if (animationAlpha > 1.0f) animationAlpha = 1.0f;

            SDL_Surface *endScreen = (questionsAnswered >= MAX_QUESTIONS) ? scaledWin : scaledLose;
            SDL_SetAlpha(endScreen, SDL_SRCALPHA, (Uint8)(animationAlpha * 255));
            SDL_BlitSurface(endScreen, NULL, screen, NULL);

            // Display score
            char scoreText[50];
            sprintf(scoreText, "Score: %d", gameState.score);
            SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText, (SDL_Color){255, 255, 255});
            SDL_Rect scoreRect = {350, 400, scoreSurface->w, scoreSurface->h};
            SDL_BlitSurface(scoreSurface, NULL, screen, &scoreRect);
            SDL_FreeSurface(scoreSurface);

            // Display restart prompt
            SDL_Surface* restartSurface = TTF_RenderText_Solid(font, "Press R to Restart", (SDL_Color){255, 255, 255});
            SDL_Rect restartRect = {350, 450, restartSurface->w, restartSurface->h};
            SDL_BlitSurface(restartSurface, NULL, screen, &restartRect);
            SDL_FreeSurface(restartSurface);
        } else if (inQuiz == 0 && inPuzzle == 0) {
            SDL_BlitSurface(background, NULL, screen, NULL);
            for (int i = 0; i < 2; i++) {
                if (i == currentHovered) {
                    SDL_BlitSurface(hoveredButtons[i].image, NULL, screen, &hoveredButtons[i].rect);
                } else {
                    SDL_BlitSurface(normalButtons[i].image, NULL, screen, &normalButtons[i].rect);
                }
            }
        } else if (inQuiz) {
            updateGameState(&gameState);
            if (gameState.timeLeft <= 0 || gameState.lives <= 0) {
                inQuiz = 0;
                gameEnded = 1;
                gameState.startTime = 0;
                for (int i = 0; i < MAX_QUESTIONS; i++) questions[i].used = 0;
                Mix_HaltMusic();
                if (loseSound) Mix_PlayChannel(-1, loseSound, 0);
            }

            SDL_BlitSurface(background, NULL, screen, NULL);
            updateTimerBar(&gameTimer, (float)gameState.timeLeft / TOTAL_QUIZ_TIME, screen);
            renderTimerBar(screen, &gameTimer);

            if (currentQuestion) {
                SDL_Surface* questionSurface = TTF_RenderText_Solid(font, currentQuestion->question, (SDL_Color){255, 255, 255});
                SDL_Rect questionRect = {100, 100, questionSurface->w, questionSurface->h};
                SDL_BlitSurface(questionSurface, NULL, screen, &questionRect);
                SDL_FreeSurface(questionSurface);
            }

            // Display score and lives
            char statusText[50];
            sprintf(statusText, "Score: %d Lives: %d", gameState.score, gameState.lives);
            SDL_Surface* statusSurface = TTF_RenderText_Solid(font, statusText, (SDL_Color){255, 255, 255});
            SDL_Rect statusRect = {10, 10, statusSurface->w, statusSurface->h};
            SDL_BlitSurface(statusSurface, NULL, screen, &statusRect);
            SDL_FreeSurface(statusSurface);

            for (int j = 2; j < NUM_BUTTONS; j++) {
                if (j == currentHovered) {
                    SDL_BlitSurface(hoveredButtons[j].image, NULL, screen, &hoveredButtons[j].rect);
                    if (hoveredButtons[j].textSurface) {
                        SDL_BlitSurface(hoveredButtons[j].textSurface, NULL, screen, &hoveredButtons[j].textRect);
                    }
                } else {
                    SDL_BlitSurface(normalButtons[j].image, NULL, screen, &normalButtons[j].rect);
                    if (normalButtons[j].textSurface) {
                        SDL_BlitSurface(normalButtons[j].textSurface, NULL, screen, &normalButtons[j].textRect);
                    }
                }
            }
        }

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    // Cleanup
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (normalButtons[i].image) SDL_FreeSurface(normalButtons[i].image);
        if (normalButtons[i].textSurface) SDL_FreeSurface(normalButtons[i].textSurface);
        if (hoveredButtons[i].image) SDL_FreeSurface(hoveredButtons[i].image);
        if (hoveredButtons[i].textSurface) SDL_FreeSurface(hoveredButtons[i].textSurface);
    }

    SDL_FreeSurface(background);
    SDL_FreeSurface(winScreen);
    SDL_FreeSurface(loseScreen);
    SDL_FreeSurface(scaledWin);
    SDL_FreeSurface(scaledLose);
    if (hoverSound) Mix_FreeChunk(hoverSound);
    if (suspenseMusic) Mix_FreeMusic(suspenseMusic);
    if (winSound) Mix_FreeChunk(winSound);
    if (loseSound) Mix_FreeChunk(loseSound);
    Mix_CloseAudio();
    freeTimerBar(&gameTimer);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
