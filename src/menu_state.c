#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"

extern Context ctx;
extern FSM fsm;


bool initMenuState(void);
void handleInputMenuState(void);
void updateMenuState(void);
void renderMenuState(void);

bool initMenuState(void)
{

    //Create the fsm state
    fsm.states[FSM_MENU_STATE].id = FSM_MENU_STATE;
    fsm.states[FSM_MENU_STATE].update = updateMenuState;
    fsm.states[FSM_MENU_STATE].handleInput = handleInputMenuState;
    fsm.states[FSM_MENU_STATE].render = renderMenuState;

    return true;
}

void handleInputMenuState(void)
{
    static SDL_Event event;
    //handle events on queue
    while(SDL_PollEvent(&event))
    {
        if((event.type == SDL_QUIT))
        {
            ctx.quit = true;
            return;
        }
        else if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_RETURN:
                    fsm.currentState = FSM_PLAY_STATE;
                    break;

                case SDLK_g:
                    break;

                case SDLK_r:
                    break;

                case SDLK_e:
                    break;

                case SDLK_ESCAPE:
                    ctx.quit = true;
                    return;
            }
        }
    }
}

void updateMenuState(void) {}

void renderMenuState(void)
{
    //First clear the renderer
    SDL_SetRenderDrawColor(ctx.pRen, 0,0,0,255);
    SDL_RenderClear(ctx.pRen);

    //Render the scene
    SDL_RenderCopy(ctx.pRen, rsmgrGetTexture(TEX_ID_MENU), NULL, NULL);

    //Now update the screen (perhaps the buffers are now switched...)
    SDL_RenderPresent(ctx.pRen);
}


