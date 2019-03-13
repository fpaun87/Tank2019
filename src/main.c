#include <stdio.h> 
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "global_defs.h"
#include "timer.h"
#include "resource_mgr.h"
#include "fsm.h"

bool appInit(void);
void appClose(void);

//The global app context
Context ctx;

//The finite state machine
FSM fsm;

extern bool initMenuState(void);
extern bool initPlayState(void);

int main(void)
{
    if(!appInit())
        return 0;

    /*
     * **** THE APPLICATION'S MAIN LOOP *****
     */
    uint32_t updateInterval = 1000u/(FPS);
    uint32_t tf = 0u; //ms
    uint32_t ti = SDL_GetTicks(); //ms

    while (!ctx.quit)
    {
        ti = SDL_GetTicks();
        fsm.states[fsm.currentState].handleInput();
        fsm.states[fsm.currentState].update();
        fsm.states[fsm.currentState].render();
        tf = SDL_GetTicks();
        if((tf - ti) < updateInterval)
            SDL_Delay(tf - ti);
    }
    //Cleanup
    appClose();
	return 0;
}

bool appInit(void)
{
    memset(&ctx, 0, sizeof(Context));

    //Initialize the various SDL susbsystems 

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) <  0)
    {
        printf("SDL failed to properly initialize: %s\n", SDL_GetError());
        return false;
    }

    //Init the audio system
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize: %s\n", Mix_GetError());
        return false;
    }

    //Hide the mouse pointer
    SDL_ShowCursor(SDL_DISABLE);

    //Create a SDL window 
    ctx.pWin = SDL_CreateWindow("Tank2019", WND_TOP_LEFT_X, WND_TOP_LEFT_Y, WND_WIDTH, WND_HEIGHT,                                                                                 SDL_WINDOW_SHOWN);
    if(!ctx.pWin)
    {
        printf("SDL window could not be created! %s\n", SDL_GetError());
        Mix_Quit();
        SDL_Quit();
        return false;
    }

    //Create a renderer 
    ctx.pRen = SDL_CreateRenderer(ctx.pWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!ctx.pRen)
    {
        SDL_DestroyWindow(ctx.pWin);
        printf("The renderer could not be created! %s\n", SDL_GetError());
        Mix_Quit();
        SDL_Quit();
        return false;
    }

    //Init SDL_ttf
    if(TTF_Init() == -1)
    {
        printf("ERROR! SDL_ttf could not be initialized!\n");
        Mix_Quit();
        SDL_DestroyRenderer(ctx.pRen);
        SDL_DestroyWindow(ctx.pWin);
        SDL_Quit();
        return false;
    }

    //Use SDL'S  GameController API
    int numOfJoysticks = SDL_NumJoysticks();
    if(numOfJoysticks < 0)
        printf("No joysticks found! %s\n", SDL_GetError());
    
    //Let's see if our joystick is supported by the GameController API
    if(!SDL_IsGameController(numOfJoysticks-1))
            printf("Your joystick is not a game controller! %s\n", SDL_GetError());

    ctx.pGameCtrl = SDL_GameControllerOpen(numOfJoysticks-1);
    if(!ctx.pGameCtrl)
        printf("Game controller could not be opened! %s\n", SDL_GetError());

    ctx.quit = false;
    ctx.players = 2;

    //init the resource manager
    if(!rsmgrInit())
    {
        SDL_DestroyRenderer(ctx.pRen);
        SDL_DestroyWindow(ctx.pWin);
        Mix_Quit();
        SDL_Quit();
        return false;
    }

    //Initialize the FSM
    memset(&fsm, 0, sizeof(FSM));

    if(!initMenuState())
        return false;

    if(!initPlayState())
        return false;

    //initScoreState();
    fsm.currentState = FSM_MENU_STATE;
    
    return true;
}


void appClose(void)
{
    rsmgrClose();
    if(ctx.pGameCtrl)
        SDL_GameControllerClose(ctx.pGameCtrl);

    //destroyPlayState();
    TTF_Quit();
    SDL_DestroyRenderer(ctx.pRen);
    SDL_DestroyWindow(ctx.pWin);
    Mix_Quit();
    SDL_Quit();
}

