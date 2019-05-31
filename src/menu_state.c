#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"

#define SPACE 64 //in pixels
#define MARKER_X 350
#define MARKER_START_Y 400
#define MARKER_STOP_Y (MARKER_START_Y + 2*(64+SPACE))
extern Config cfg;
extern FSM fsm;
extern bool quit;
static SDL_Rect dstRect = {MARKER_X, MARKER_START_Y, 64, 64};


bool initMenuState(void);
void handleInputMenuState(void);
void runMenuState(void);
void renderMenuState(void);

bool initMenuState(void)
{
    //Create the fsm state
    fsm.states[FSM_MENU_STATE].run = runMenuState;

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
            quit = true;
            return;
        }
        else if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_RETURN:
				/* Let's see what the user has chosen */
				switch(dstRect.y)
				{
					/* 1 player */
					case MARKER_START_Y:
						cfg.players = 1;
						cfg.p1.lives = 2;
						cfg.p1.score = 0;
						cfg.p2.lives = 0;
						cfg.p2.score = 0;
						break;

					/* 2 players */
					case (MARKER_START_Y + 64 + SPACE):
						cfg.players = 2;
						cfg.p1.lives = 2;
						cfg.p1.score = 0;
						cfg.p2.lives = 2;
						cfg.p2.score = 0;
						break;

					/* exit */
					case MARKER_STOP_Y:
						quit = true;
						return ;

					default:
						break;
				}
				fsm.currentState = FSM_LEVEL_STATE;
                    break;

                case SDLK_DOWN:
					if(dstRect.y < MARKER_STOP_Y)
						dstRect.y += (dstRect.w + SPACE);

                    break;

                case SDLK_UP:
					if(dstRect.y > MARKER_START_Y)
						dstRect.y -= (dstRect.w + SPACE);

                    break;

                case SDLK_ESCAPE:
                    quit = true;
                    return;

				default :
					break;
            }
        }
    }
}


void renderMenuState(void)
{
    //First clear the renderer
    SDL_SetRenderDrawColor(cfg.pRen, 0,0,0,255);
    SDL_RenderClear(cfg.pRen);

    //Render the scene
    SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_MENU), NULL, NULL);

	//Render the marker
	SDL_RenderCopyEx(cfg.pRen, rsmgrGetTexture(TEX_ID_PLAYER1_LEVEL1), NULL, &dstRect, 90, NULL, SDL_FLIP_NONE);	

	//Print the options
	printfg(TEX_ID_MENU_FONT, MARKER_X + 64 + 32, MARKER_START_Y, "1 PLAYER");
	printfg(TEX_ID_MENU_FONT, MARKER_X + 64 + 32, MARKER_START_Y + 64 + SPACE, "2 PLAYERS");
	printfg(TEX_ID_MENU_FONT, MARKER_X + 64 + 32, MARKER_START_Y + 2*(64 + SPACE), "EXIT");
    //Now update the screen (perhaps the buffers are now switched...)
    SDL_RenderPresent(cfg.pRen);
}


void runMenuState(void) 
{
	handleInputMenuState();


	renderMenuState();
}
