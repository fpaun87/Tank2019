#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"

#define TEXT_SPEED 4

extern Config cfg;
extern FSM fsm;
extern void playSound(Tank *pTank, int soundId);

bool initGameOverState(void);
void handleInputGameOverState(void);
void runGameOverState(void);
void pre_runGameOverState(void);
void renderGameOverState(void);
extern void renderPlayState(void);

/* The current height of the text */
static int y = 0;

bool initGameOverState(void)
{
    //Create the fsm state
    fsm.states[FSM_GAMEOVER_STATE].run = pre_runGameOverState;
    return true;
}

void handleInputGameOverState(void)
{
    static SDL_Event event;
    //handle events on queue
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
				case SDLK_RETURN:
                    fsm.currentState = FSM_MENU_STATE;
					fsm.states[FSM_GAMEOVER_STATE].run = pre_runGameOverState;
                    break;

            }
        }
    }
}


void renderGameOverState(void)
{
    //First clear the renderer
    SDL_RenderClear(cfg.pRen);
	renderPlayState();
	printfg(TEX_ID_GAMEOVER_FONT, (WND_WIDTH - strlen("GAME OVER") * 90)/2,
			y, "GAME OVER");

    //Now update the screen (perhaps the buffers are now switched...)
    SDL_RenderPresent(cfg.pRen);
}


void runGameOverState(void) 
{
	handleInputGameOverState();
	if(y > ((WND_HEIGHT - 100)/2))
		y -= TEXT_SPEED;

	renderGameOverState();
}

void pre_runGameOverState(void)
{
	y = WND_HEIGHT - 100;
    fsm.states[FSM_GAMEOVER_STATE].run = runGameOverState;
	Tank t;
	t.driver = HUMAN_DRIVER;
	playSound(&t, CHUNK_ID_GAME_OVER);
}
