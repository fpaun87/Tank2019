#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define WND_TOP_LEFT_X 0
#define WND_TOP_LEFT_Y 0
#define WND_WIDTH 1312
#define WND_HEIGHT 832
#define SCENE_TOP_LEFT_X 240
#define SCENE_TOP_LEFT_Y WND_TOP_LEFT_Y
#define SCENE_WIDTH WND_HEIGHT
#define SCENE_HEIGHT WND_HEIGHT
#define FPS (uint32_t) 30u

enum MoveEvent {ME_STOP, ME_LEFT, ME_DOWN, ME_UP, ME_RIGHT};
enum FireEvent {FE_NONE, FE_FIRE};
enum FSMStateId {
    FSM_MENU_STATE,
    FSM_LEVEL_STATE,
    FSM_PLAY_STATE,
    FSM_PAUSE_STATE,
    FSM_GAMEOVER_STATE
};

typedef struct Config {
    SDL_Window *pWin;
    SDL_Renderer *pRen;
    SDL_GameController *pGameCtrl;
    int players;
    int enemiesLeft;
    int Level;
} Config;


#endif //GLOBAL_DEFS_H
