#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"

#define MIN_LEVEL 1
#define MAX_LEVEL 50

extern Config cfg;
extern FSM fsm;

bool initLevelState(void);
void handleInputLevelState(void);
void runLevelState(void);
void renderLevelState(void);

static SDL_Rect rect = {WND_TOP_LEFT_X, WND_TOP_LEFT_Y, WND_WIDTH, WND_HEIGHT};

bool initLevelState(void)
{
    //Create the fsm state
    fsm.states[FSM_LEVEL_STATE].run = runLevelState;
	cfg.Level = MIN_LEVEL;
    return true;
}

void handleInputLevelState(void)
{
    static SDL_Event event;
    //handle events on queue
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_RETURN:
                    fsm.currentState = FSM_PLAY_STATE;
                    break;

				case SDLK_DOWN:
					cfg.Level--;
					if(cfg.Level < MIN_LEVEL)
						cfg.Level = MAX_LEVEL;

					break;

				case SDLK_UP:
					cfg.Level++;
					if(cfg.Level > MAX_LEVEL)
						cfg.Level = MIN_LEVEL;

					break;

            }
        }
    }
}


void renderLevelState(void)
{
    //First clear the renderer
    SDL_SetRenderDrawColor(cfg.pRen, 128,128,128,255);
    SDL_RenderClear(cfg.pRen);
	SDL_RenderFillRect(cfg.pRen, &rect);
	printfg(TEX_ID_LEVEL_FONT, (WND_WIDTH - strlen("LEVEL 01") * 64)/2,
			(WND_HEIGHT - 64)/2, "LEVEL %02d", cfg.Level);

    //Now update the screen (perhaps the buffers are now switched...)
    SDL_RenderPresent(cfg.pRen);
}


void runLevelState(void) 
{

	handleInputLevelState();
	renderLevelState();
}
