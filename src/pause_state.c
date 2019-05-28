#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"
#include "play_state.h"
#include "timer.h"

#define BLINK_INTERVAL_MS 500 //in seconds

extern Config cfg;
extern FSM fsm;
extern Tank tank_array[MAX_TANKS];
extern ScoreLabel scoreLabelArray[MAX_SCORE_LABELS];

bool initPauseState(void);
void handleInputPauseState(void);
void runPauseState(void);
void renderPauseState(void);
void pre_runPauseState(void);
extern void renderPlayState(void);

static bool renderText = true;
static Timer timer = {0, 0, 0, 0, 0};

bool initPauseState(void)
{
    //Create the fsm state
    fsm.states[FSM_PAUSE_STATE].run = pre_runPauseState;
    return true;
}

void handleInputPauseState(void)
{
    static SDL_Event event;
    //handle events on queue
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
				/*
                case SDLK_ESCAPE:
                    fsm.currentState = FSM_MENU_STATE;
                    break;
				*/
				case SDLK_p:
					//Resume all the timers
					for(int i = 0; i < MAX_TANKS; i++)
					{
						resumeTimer(&tank_array[i].timer1);
						resumeTimer(&tank_array[i].timer2);
					}

					//Pause the score label timers
					for(int i = 0; i < MAX_SCORE_LABELS; i++)
					{
						resumeTimer(&scoreLabelArray[i].timer);
					}

                    fsm.currentState = FSM_PLAY_STATE;
					Mix_ResumeMusic();
					break;

            }
			fsm.states[FSM_PAUSE_STATE].run = pre_runPauseState;
        }
    }
}


void renderPauseState(void)
{
	static SDL_Rect label = {
		(WND_WIDTH - (strlen("PAUSE")+1)*64)/2,
		(WND_HEIGHT - 2*64)/2,
		(strlen("PAUSE")+1)*64,
		2*64 };

    //First clear the renderer
    SDL_RenderClear(cfg.pRen);
	renderPlayState();

	if(renderText)
	{
		SDL_SetRenderDrawColor(cfg.pRen, 0,0,0,255);
		SDL_RenderFillRect(cfg.pRen, &label);
		printfg(TEX_ID_PAUSE_FONT, (WND_WIDTH - strlen("PAUSE") * 64)/2,
			(WND_HEIGHT - 64)/2, "PAUSE");
	}

    //Now update the screen (perhaps the buffers are now switched...)
    SDL_RenderPresent(cfg.pRen);
}


void runPauseState(void) 
{
	if(isTimerUp(&timer))
	{
		renderText = !renderText;
		setTimer(&timer, BLINK_INTERVAL_MS);
	}
	handleInputPauseState();
	renderPauseState();
}

void pre_runPauseState(void)
{
	//Set the timer
	setTimer(&timer, BLINK_INTERVAL_MS);
	renderText = true;

	//Pause all the tank timers
    for(int i = 0; i < MAX_TANKS; i++)
	{
		pauseTimer(&tank_array[i].timer1);
		pauseTimer(&tank_array[i].timer2);
	}	

	//Pause the score label timers
	for(int i = 0; i < MAX_SCORE_LABELS; i++)
	{
		pauseTimer(&scoreLabelArray[i].timer);
	}

	fsm.states[FSM_PAUSE_STATE].run = runPauseState;
}

