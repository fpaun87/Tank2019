#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define WND_TOP_LEFT_X 0
#define WND_TOP_LEFT_Y 0
#define WND_WIDTH 1200
#define WND_HEIGHT 960
#define SCENE_TOP_LEFT_X WND_TOP_LEFT_X
#define SCENE_TOP_LEFT_Y WND_TOP_LEFT_Y
#define SCENE_WIDTH 960
#define SCENE_HEIGHT WND_HEIGHT
#define FPS (uint32_t) 30u

enum MoveEvent {ME_STOP, ME_LEFT, ME_DOWN, ME_UP, ME_RIGHT};
enum FireEvent {FE_NONE, FE_FIRE};

typedef struct Context{
    bool quit;
    SDL_Window *pWin;
    SDL_Renderer *pRen;
    SDL_GameController *pGameCtrl;
    int players;
}Context;

typedef struct Gameplay{
    bool gameOver;
    int enemiesLeft;
    int Level;
}Gameplay;

#endif //GLOBAL_DEFS_H
