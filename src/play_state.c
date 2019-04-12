#include "play_state.h"
#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"
#include "queue.h"

extern FSM fsm;
extern Config cfg;
extern bool quit;

void runPlayState(void);
void setTankLevel(Tank* pTank, int level);
void handleBulletTankCollision(Bullet *pBullet, Tank *pTank);
void playerTankHitByEnemyBullet(Tank *pTank);
void enemyTankHitByPlayerBullet(Tank *pTank);
bool activateScoreLabel(Tank *pTank);
void renderScoreLabelArray(void);
void initScoreLabelArray(void);
void pre_runPlayState(void);
void goToGameOverState(void);
void drawForest(void);

/* Terrain building functions */
void buildTerrain1(void);

/* The array of function pointers that point to
 * functions which build the terrain for each level
 */
typedef void (*BuildTerrainFuncPtr) (void);
BuildTerrainFuncPtr terrainBuilders[50] = { buildTerrain1, 0 };

Tank tank_array[MAX_TANKS];
Player p1 = {2, 0, &tank_array[0]};
Player p2 = {2, 0, &tank_array[1]};
Bullet bullet_array[MAX_BULLETS];
SDL_Rect scene;
TerrainTile map[MAX_TERRAIN_TILES];
ScoreLabel scoreLabelArray[MAX_SCORE_LABELS];
BonusHandlerFuncPtr bonusHandlerArray[7];
Bonus bonus; 


/* Prototypes for the bonus functions */
void initBonus(void);
/*
 * void handleBonusStar(Tank *pTank);
 * void handleBonusTank(Tank *pTank);
 * void handleBonusHelmet(Tank *pTank);
 * void handleBonusGun(Tank *pTank);
 * void handleBonusBomb(Tank *pTank);
 * void handleBonusClock(Tank *pTank);
 * void handleBonusShovel(Tank *pTank);
 * void handleBonusShip(Tank *pTank);
 */
void renderBonus(void);

void updateTanks(void)
{
    static SDL_Rect* pNewPos;
    static Tank *pTank = NULL; //for convenience
    bool outOfScene = false;
    bool hitsTank = false;
    bool hitsTerrain = false;

    for(int i = 0; i < MAX_TANKS; i++)
    {
        outOfScene = false;
        hitsTank = false;
        hitsTerrain = false;
        pTank = &tank_array[i];
        if(!pTank->enabled)
            continue;

        /*
         * Only read your new move event if your (x,y) is a 
         * multiple of 16. This is for easy tank maneuvering
         */
        if(!(pTank->rect.x % 16) && !(pTank->rect.y % 16))
            pTank->currMe = pTank->newMe;


        if(pTank->currMe == ME_STOP)
        {
            fireTank(pTank);
            continue;
        }

        pNewPos = moveTank(pTank);

        //don't go out of bounds
        if(!isInScene(pNewPos))
            outOfScene = true;

        //check collision with the other tanks
        for(int j = 0; j < MAX_TANKS; j++)
        {
            if(i == j)
                continue;

            if(!tank_array[j].enabled)
                continue;

            if(SDL_HasIntersection(pNewPos, &tank_array[j].rect) == SDL_TRUE)
            {
                hitsTank = true;
                break;
            }
        }

        //check collision with terrain
        for(int m = 0; m < MAX_TERRAIN_TILES; m++)
        {
            if((map[m].pTex == NULL) || (map[m].type == TERRAIN_ICE) || (map[m].type == TERRAIN_FOREST)) 
                continue;

            if((map[m].type == TERRAIN_WATER) && pTank->hasBoat)
                continue;

            if(SDL_HasIntersection(pNewPos, &map[m].rect) == SDL_TRUE)
            {
                hitsTerrain = true;
                break;
            }
        }
                
        if(!outOfScene && !hitsTank && !hitsTerrain)
        {
            pTank->rect.x = pNewPos->x;
            pTank->rect.y = pNewPos->y;
            
            
        }

        //Even if we don't update the tank position, we alwasys update the orientation
        switch(pTank->currMe)
        {
           case ME_STOP:
               break;

           case ME_UP:
               pTank->angle = 0.0f;
               break;
         
           case ME_LEFT:
               pTank->angle = 270.0f;
               break;
         
           case ME_DOWN:
               pTank->angle = 180.0f;
               break;
        
           case ME_RIGHT:
               pTank->angle = 90.0f;
               break;

           default:
               printf("FATAL ERROR: %s, %d\n", __FUNCTION__, __LINE__);
               break;
        } 
        fireTank(pTank);

    }
}

bool initPlayState(void)
{
    //Init the scene
    scene.x = SCENE_TOP_LEFT_X;
    scene.y = SCENE_TOP_LEFT_Y;
    scene.w = SCENE_WIDTH;
    scene.h = SCENE_HEIGHT;


    //init the score label array
    initScoreLabelArray();

    //Create the fsm state
	FSMState state;
    state.run = pre_runPlayState;
	memcpy(&fsm.states[FSM_PLAY_STATE], &state, sizeof(FSMState));

    return true;
}

void handleInputPlayState(void)
{
    static SDL_Event event;
    //handle events on queue
    while(SDL_PollEvent(&event))
    {
        if((event.type == SDL_QUIT))
        {
            //quit = true;
            fsm.currentState = FSM_MENU_STATE;
			fsm.states[FSM_PLAY_STATE].run = pre_runPlayState;
            return;
        }

        if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
					Mix_HaltMusic();
					//quit = true;
					fsm.currentState = FSM_MENU_STATE;
					fsm.states[FSM_PLAY_STATE].run = pre_runPlayState;
					return;

                case SDLK_p:
					Mix_PauseMusic();
					fsm.currentState = FSM_PAUSE_STATE;
					return;
            
            }
        }
    }

    //Let's query the input devices
    moveTankByKeyboard(p1.pTank);
    moveTankByGamepad(p2.pTank);
}

void runPlayState(void)
{
	handleInputPlayState();

	updateTanks();
	updateBullets();

	renderPlayState();

    //Now update the screen (perhaps the buffer are now switched...)
    SDL_RenderPresent(cfg.pRen);
}

void renderPlayState(void)
{
	static SDL_Rect icon = {1080,0,32,32};
    //First clear the renderer
    SDL_SetRenderDrawColor(cfg.pRen, 128,128,128,255);
    SDL_RenderClear(cfg.pRen);
    //Render the scene
    SDL_SetRenderDrawColor(cfg.pRen, 0,0,0,255);
    SDL_RenderFillRect(cfg.pRen, &scene);
    //Draw the terrain
    drawTerrain();
    //Render the tanks
    renderTanks();
    //Render all the enabled bullets
    renderBullets();
	//Rendere the forest
	drawForest();
    //Render the bonus
    renderBonus();
    //Render the score labels
    renderScoreLabelArray();
    //Render the text
    printfg(TEX_ID_PLAY_FONT, 1080, 400, "P I\n");
	icon.y = 430;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_HEALTH), NULL, &icon); 
    printfg(TEX_ID_PLAY_FONT, 1126, 430, "%02d", p1.lives);
	icon.y = 470;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_COIN), NULL, &icon); 
    printfg(TEX_ID_PLAY_FONT, 1126, 470, "%06d", p1.score);
	printfg(TEX_ID_PLAY_FONT, 1080, 530, "P II");
	icon.y = 560;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_HEALTH), NULL, &icon); 
	printfg(TEX_ID_PLAY_FONT, 1126, 560, "%02d", p2.lives);
	icon.y = 600;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_COIN), NULL, &icon); 
	printfg(TEX_ID_PLAY_FONT, 1126, 600, "%06d", p2.score);

	icon.y = 700;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_FLAG), NULL, &icon); 
    printfg(TEX_ID_PLAY_FONT, 1126, 700, "%02d", cfg.Level);

}

void moveTankByKeyboard(Tank *pTank)
{
    static const Uint8* currentKeyStates = NULL;

    currentKeyStates = SDL_GetKeyboardState(NULL);


    if(currentKeyStates [SDL_SCANCODE_SPACE]){
        pTank->fe = FE_FIRE;
    }

    if(currentKeyStates [SDL_SCANCODE_UP]){
        pTank->newMe = ME_UP;
        return;
    }

    if(currentKeyStates [SDL_SCANCODE_LEFT]){
        pTank->newMe = ME_LEFT;
        return;
    }

    if(currentKeyStates [SDL_SCANCODE_DOWN]){
        pTank->newMe = ME_DOWN;
        return;
    }

    if(currentKeyStates [SDL_SCANCODE_RIGHT]){
        pTank->newMe = ME_RIGHT;
        return;
    }

    //When none of the movement keys are pressed, stop the tank
    pTank->newMe = ME_STOP;
}

void moveTankByGamepad(Tank *pTank)
{
    if(SDL_GameControllerGetButton(cfg.pGameCtrl, SDL_CONTROLLER_BUTTON_A)){
        pTank->fe = FE_FIRE;
    }

    if(SDL_GameControllerGetButton(cfg.pGameCtrl, SDL_CONTROLLER_BUTTON_DPAD_UP)){
        pTank->newMe = ME_UP;
        return;
    }

    if(SDL_GameControllerGetButton(cfg.pGameCtrl, SDL_CONTROLLER_BUTTON_DPAD_LEFT)){
        pTank->newMe = ME_LEFT;
        return;
    }

    if(SDL_GameControllerGetButton(cfg.pGameCtrl, SDL_CONTROLLER_BUTTON_DPAD_DOWN)){
        pTank->newMe = ME_DOWN;
        return;
    }

    if(SDL_GameControllerGetButton(cfg.pGameCtrl, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)){
        pTank->newMe = ME_RIGHT;
        return;
    }

    //When none of the movement buttons are pressed, just stop the tank
    pTank->newMe = ME_STOP;
}


void handleGameOver(void)
{
    return;
}

SDL_Rect* moveTank(Tank *pTank )
{
    static SDL_Rect rect;
    rect.x = pTank->rect.x;
    rect.y = pTank->rect.y;
    rect.w = pTank->rect.w;
    rect.h = pTank->rect.h;

    switch(pTank->currMe)
    {
       case ME_STOP:
           return &rect;

       case ME_UP:
           rect.y -= pTank->speed;
           break;
     
       case ME_LEFT:
           rect.x -=  pTank->speed;
           break;
     
       case ME_DOWN:
           rect.y += pTank->speed;
           break;
    
       case ME_RIGHT:
           rect.x += pTank->speed;
           break;

       default:
           printf("FATAL ERROR: %s, %d\n", __FUNCTION__, __LINE__);
           break;
    } 

    return &rect;
}

void fireTank(Tank *pTank)
{
    int i = 0;

    //Only for convenience
    int bw = bullet_array[0].rect.w;
    int tw = pTank->rect.w;

    if(pTank->fe == FE_NONE)
        return;

    //You can't fire when the timer is ticking
    if(!isTimerUp(pTank->pTimer))
    {
        pTank->fe = FE_NONE;
        return;
    }

    //Find a bullet that is not enabled
    for(i = 0; i < MAX_BULLETS; i++)
    {
        if(!bullet_array[i].enabled)
            break;
    }

    //No free bullet found? That's so sad! :(
    if(i == MAX_BULLETS)
    {
        printf("FATAL ERROR: NO MORE BULLETS AVAILABLE! FIX YOUR LOGIC!\n");
        pTank->fe = FE_NONE;
        return;
    }

    bullet_array[i].angle = pTank->angle;

    switch((int)pTank->angle)
    {
        case 0:
            bullet_array[i].rect.x = pTank->rect.x + (tw - bw)/2;
            bullet_array[i].rect.y = pTank->rect.y - bw; 
            break;

        case 90:
            bullet_array[i].rect.x = pTank->rect.x + tw;
            bullet_array[i].rect.y = pTank->rect.y + (tw - bw)/2;
            break;

        case 180:
            bullet_array[i].rect.x = pTank->rect.x + (tw - bw)/2;
            bullet_array[i].rect.y = pTank->rect.y + tw;
            break;

        case 270:
            bullet_array[i].rect.x = pTank->rect.x - bw;
            bullet_array[i].rect.y = pTank->rect.y + (tw - bw)/2;
            break;

        //That's very bad
        default:
            printf("FATAL ERROR: %s, %d\n", __FUNCTION__, __LINE__);
            break;
    }
    bullet_array[i].enabled = true;
    bullet_array[i].tankId = pTank->id;
    pTank->fe = FE_NONE;

    //reset the fire timer after each successfull shot
    setTimer(pTank->pTimer, pTank->fireHoldout);

    //Play the fire sound
    Mix_PlayChannel(-1, rsmgrGetChunk(CHUNK_ID_FIRE), 0);
}

bool initTank(Tank *pTank, int level, int x, int y, float angle, enum TankID id)
{
    memset(pTank, 0, sizeof(Tank));

    setTankLevel(pTank, level);
    pTank->rect.x = x;
    pTank->rect.y = y;
    pTank->speed = DEFAULT_TANK_SPEED; //pixels per frame
    pTank->angle = angle;
    pTank->pTimer = &pTank->timer;
    pTank->newMe = ME_STOP;
    pTank->currMe = ME_STOP;
    pTank->fe = FE_NONE;
    pTank->enabled = true;
    pTank->id = id;
    SDL_QueryTexture(pTank->pTex, NULL, NULL, &pTank->rect.w, &pTank->rect.h);
    return true;
}

bool initBullets(void)
{
    //The bullet texture's width and height
    int w=0, h=0;
    memset(bullet_array, 0, sizeof(bullet_array));

    //Aquire the bullet texture from the resource manager
    SDL_Texture *pTex = rsmgrGetTexture(TEX_ID_BULLET);
    SDL_QueryTexture(pTex, NULL, NULL, &w, &h);

    for(int i = 0; i < MAX_BULLETS; i++)
    {
        bullet_array[i].rect.w = w; 
        bullet_array[i].rect.h = h;
        bullet_array[i].pTex = pTex;
        bullet_array[i].speed = DEFAULT_BULLET_SPEED; //in pixels per update time
        bullet_array[i].enabled = false;
    }
    return true;
}

void updateBullets()
{
    bool hitsSomething = false;

    for(int i = 0; i < MAX_BULLETS; i++)
    {
        if(!bullet_array[i].enabled)
            continue;

        //Move the bullet
        switch((int)bullet_array[i].angle)
        {
            case 0:
                bullet_array[i].rect.y -= bullet_array[0].speed;
                break;

            case 90:
                bullet_array[i].rect.x += bullet_array[0].speed;
                break;

            case 180:
                bullet_array[i].rect.y += bullet_array[0].speed;
                break;

            case 270:
                bullet_array[i].rect.x -= bullet_array[0].speed;
                break;

            default:
                printf("FATAL ERROR: %s, %d\n", __FUNCTION__, __LINE__);
        }

        //Destroy the bullets that get out of scene
        if(!isInScene(&bullet_array[i].rect))
        {
            bullet_array[i].enabled = false;
            break;
        }

        //Check collisions with tanks
        for(int m  = 0; m < MAX_TANKS; m++)
        {
            if(!tank_array[m].enabled)
                continue;

            if(SDL_HasIntersection(&bullet_array[i].rect, &tank_array[m].rect) != SDL_TRUE)
                continue;

            hitsSomething = true;

            //handle bullet-tank collision
            handleBulletTankCollision(&bullet_array[i], &tank_array[m]);
        }

        /*Destroy the bullets that hit other bullets*/
        for(int j = 0; j < MAX_BULLETS; j++)
        {
            if(i == j)
                continue;

            if(!bullet_array[j].enabled)
                continue;

            if(SDL_HasIntersection(&bullet_array[i].rect, &bullet_array[j].rect) == SDL_TRUE)
            {
                bullet_array[j].enabled = false;
                hitsSomething = true;
            }
        }

        //Destroy bullets that hit terrain.
        for(int k = 0; k < MAX_TERRAIN_TILES; k++)
        {
            if((map[k].type != TERRAIN_BRICK) && (map[k].type != TERRAIN_SHIELD) && (map[k].type != TERRAIN_EAGLE))
                continue;

            if(SDL_HasIntersection(&bullet_array[i].rect, &map[k].rect) == SDL_TRUE)
            {
                if(map[k].type == TERRAIN_EAGLE)
                {
                    map[k].pTex = rsmgrGetTexture(TEX_ID_DEAD_EAGLE);
                    bullet_array[i].enabled = false;
					goToGameOverState();
					return;
                }

                map[k].pTex = NULL;
                map[k].type = TERRAIN_NONE;
                hitsSomething = true;
            }
            
        }

        if(hitsSomething)
            bullet_array[i].enabled = false;

        hitsSomething = false;
            
    }
}

void renderTanks(void)
{
    for(int i = 0; i < MAX_TANKS; i++)
    {
        if(tank_array[i].enabled)
            SDL_RenderCopyEx(cfg.pRen, tank_array[i].pTex, NULL, &tank_array[i].rect, 
                            tank_array[i].angle, NULL, SDL_FLIP_NONE);
    }
}

void renderBullets(void)
{
    static SDL_Rect rect;

    for(int i = 0; i < MAX_BULLETS; i++)
    {
        if(!bullet_array[i].enabled)
            continue;

        rect.x = bullet_array[i].rect.x;
        rect.y = bullet_array[i].rect.y;
        rect.w = bullet_array[i].rect.w;
        rect.h = bullet_array[i].rect.h;

        SDL_RenderCopyEx(cfg.pRen, bullet_array[i].pTex, NULL, &rect, bullet_array[i].angle, NULL, SDL_FLIP_NONE);
    }
}

bool isInScene(SDL_Rect* pRect)
{
    if(pRect->x < SCENE_TOP_LEFT_X || pRect->x > SCENE_TOP_LEFT_X + SCENE_WIDTH - pRect->w ||
       pRect->y < SCENE_TOP_LEFT_Y || pRect->y > SCENE_TOP_LEFT_Y + SCENE_HEIGHT - pRect->h)
        return false;
    else
        return true;

}

bool initTankArray(void)
{
    memset(tank_array, 0, sizeof(tank_array));

    if(!initTank(&tank_array[0], 4, SCENE_TOP_LEFT_X + 4*64, SCENE_HEIGHT-64, 0.0f, TANK_ID_PLAYER1))
        return false;

    if(!initTank(&tank_array[1], 1, SCENE_TOP_LEFT_X + 8*64, SCENE_HEIGHT-64, 0.0f, TANK_ID_PLAYER2))
        return false;
    
    if(cfg.players == 1)
        tank_array[1].enabled = false; 
    
    //Init the first batch of enemy tanks
    initTank(&tank_array[2], 4, SCENE_TOP_LEFT_X, SCENE_TOP_LEFT_Y, 180.0f, TANK_ID_ENEMY);
    initTank(&tank_array[3], 4, SCENE_TOP_LEFT_X + 6*64, SCENE_TOP_LEFT_Y, 180.0f, TANK_ID_ENEMY);
    initTank(&tank_array[4], 4, SCENE_TOP_LEFT_X + 12*64, SCENE_TOP_LEFT_Y, 180.0f, TANK_ID_ENEMY);
    initTank(&tank_array[5], 4, SCENE_TOP_LEFT_X + 6*64, SCENE_TOP_LEFT_Y + 2*64, 180.0f, TANK_ID_ENEMY);

    return true;
}

void buildTerrain1(void)
{
    int i = 0;
    memset(map, 0, sizeof(map));

	//For now level 1 map from Tank 1990 N

	//Line 1: all ice:
    for(int x = SCENE_TOP_LEFT_X; x < SCENE_TOP_LEFT_X + SCENE_WIDTH; x += 64)
	{
            map[i].rect.x = x;
            map[i].rect.y = 0;
            map[i].rect.w = 64;
            map[i].rect.h = 64; 
            map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
            map[i].type = TERRAIN_ICE;
            i++;
	}
	
	//Line 2
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 4*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 8*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 1*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	//Line 3
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 2*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	//Line 4
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i ++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 4*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i ++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 3*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	//Line 5
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i += 3;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 4*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	//Line 6
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 4*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 8*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 5*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
	map[i].type = TERRAIN_ICE;
	i++;

	//Line 7: all ice
    for(int x = SCENE_TOP_LEFT_X; x < SCENE_TOP_LEFT_X + SCENE_WIDTH; x += 64)
	{
            map[i].rect.x = x;
            map[i].rect.y = 6*64;
            map[i].rect.w = 64;
            map[i].rect.h = 64; 
            map[i].pTex = rsmgrGetTexture(TEX_ID_ICE);
            map[i].type = TERRAIN_ICE;
            i++;
	}

    //Line 8
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 7*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i += 4;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 7*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 6*64;
	map[i].rect.y = 7*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 7*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 4;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 7*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i++;

	//Line 9
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 8*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 8*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 8*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 8*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 6*64;
	map[i].rect.y = 8*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 8*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 8*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 8*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 8*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	//Line 10
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 2*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 4*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 6*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 8*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 10*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_SHIELD);
	map[i].type = TERRAIN_SHIELD;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 9*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	//Line 11
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 10*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	//Line 12
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 11*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 11*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 3*64;
	map[i].rect.y = 11*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64 + 32;
	map[i].rect.y = 11*64 + 32;
	map[i].rect.w = 32;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 6*64;
	map[i].rect.y = 11*64 + 32;
	map[i].rect.w = 64;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 11*64 + 32;
	map[i].rect.w = 32;
	map[i].rect.h = 32; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 9*64;
	map[i].rect.y = 11*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 11*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 12*64;
	map[i].rect.y = 11*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	//Line 13
	map[i].rect.x = SCENE_TOP_LEFT_X + 0;
	map[i].rect.y = 12*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 1*64;
	map[i].rect.y = 12*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 2*64;
	map[i].rect.y = 12*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i += 3;

	map[i].rect.x = SCENE_TOP_LEFT_X + 5*64 + 32;
	map[i].rect.y = 12*64;
	map[i].rect.w = 32;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i++;

	//THE MIGHTY EAGLE
	map[i].rect.x = SCENE_TOP_LEFT_X + 6*64;
	map[i].rect.y = 12*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_EAGLE);
	map[i].type = TERRAIN_EAGLE;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 7*64;
	map[i].rect.y = 12*64;
	map[i].rect.w = 32;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_BRICK);
	map[i].type = TERRAIN_BRICK;
	i += 2;

	map[i].rect.x = SCENE_TOP_LEFT_X + 10*64;
	map[i].rect.y = 12*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
	i++;

	map[i].rect.x = SCENE_TOP_LEFT_X + 11*64;
	map[i].rect.y = 12*64;
	map[i].rect.w = 64;
	map[i].rect.h = 64; 
	map[i].pTex = rsmgrGetTexture(TEX_ID_FOREST);
	map[i].type = TERRAIN_FOREST;
}

bool isInvalidMapLocation(int x, int y)
{
    //chec for the upper 3 tanks
    if(x == 0 || x == 32 || x == 14*32 || x == 15*32 || x == 28*32 || x == 29*32)
    {
        if(y == 0 || y == 32)
        {
            return true;
        }
    }

    //check for the middle tank
    if(x == 14*32 || x == 15*32)
        if(y == 14*32 || y == 15*32)
            return true;

    //check for the garrison and the players' tanks
    if(x >= 10*32 && x <= 19*32)
    {
        if(y == (SCENE_HEIGHT - 64) || y == (SCENE_HEIGHT - 32))
        {
            return true;
        }
    }

    if(x >= 13*32 && x <= 16*32 && y == SCENE_HEIGHT -96)
        return true;

    return false;
}

void drawTerrain(void)
{
    static SDL_Rect rect = {0};
    for(int i = 0; i < MAX_TERRAIN_TILES; i++)
    {
        if((map[i].pTex == NULL) || (map[i].type == TERRAIN_FOREST))
            continue;

		rect.w = map[i].rect.w;
		rect.h = map[i].rect.h;
        SDL_RenderCopy(cfg.pRen, map[i].pTex, &rect, &map[i].rect);
    }
}

void drawForest(void)
{
    static SDL_Rect rect = {0};
    for(int i = 0; i < MAX_TERRAIN_TILES; i++)
    {
        if(map[i].type != TERRAIN_FOREST)
            continue;

		rect.w = map[i].rect.w;
		rect.h = map[i].rect.h;
        SDL_RenderCopy(cfg.pRen, map[i].pTex, &rect, &map[i].rect);
    }
}


void setTankLevel(Tank* pTank, int level)
{
    int texId;
    pTank->level = level;
    pTank->hp = level;
    switch(level)
    {
        case 1:
        pTank->fireHoldout = 500;
            break;

        case 2:
        pTank->fireHoldout = 500;
            break;

        case 3:
        pTank->fireHoldout = 200;
            break;

        case 4:
        pTank->fireHoldout = 200;
            break;

        default:
            printf("YOU REALLY SCREWED THINGS UP! %s, %d\n", __FUNCTION__, __LINE__);
            return;
    }

    if(pTank == p1.pTank)
        texId = TEX_ID_PLAYER1_LEVEL1;

    
    if(pTank == p2.pTank)
        texId = TEX_ID_PLAYER2_LEVEL1;

    if((pTank != p1.pTank) && (pTank != p2.pTank))
        texId = TEX_ID_ENEMY_LEVEL1;

    texId += (level - 1);
    pTank->pTex = rsmgrGetTexture(texId);
}

void handleBulletTankCollision(Bullet *pBullet, Tank *pTank)
{
    //If 2 enemies shoot eachother nothing happens
    if((pBullet->tankId == TANK_ID_ENEMY) && (pTank->id == TANK_ID_ENEMY))
        return;

    //If the players shoot eachother nothing happens
    if((pBullet->tankId != TANK_ID_ENEMY) && (pTank->id != TANK_ID_ENEMY))
        return;

    //If a player is hit by an enemy bullet
    if((pBullet->tankId == TANK_ID_ENEMY) && (pTank->id != TANK_ID_ENEMY))
        return playerTankHitByEnemyBullet(pTank);

    //If an enemy is hit by a player's bullet
    if((pBullet->tankId != TANK_ID_ENEMY) && (pTank->id == TANK_ID_ENEMY))
        return enemyTankHitByPlayerBullet(pTank);

}

void playerTankHitByEnemyBullet(Tank *pTank)
{
    pTank->hp--;

    //If the tank is still alive
    if(pTank->hp)
    {
        setTankLevel(pTank, pTank->hp);
        return;
    }

    //If hp drops to zero, so the player tank is dead
    pTank->enabled = false;
    if(pTank->id == TANK_ID_PLAYER1)
    {
        p1.lives--;
        return;
    }


    if(pTank->id == TANK_ID_PLAYER2)
    {
        p2.lives--;
        return;
    }
}

void enemyTankHitByPlayerBullet(Tank *pTank)
{
    pTank->hp--;

    //If the tank is till alive
    if(pTank->hp)
        return;

    pTank->enabled = false;
    cfg.enemiesLeft--;

    if(pTank->id == TANK_ID_PLAYER1)
        p1.score += pTank->level * 100;


    if(pTank->id == TANK_ID_PLAYER2)
        p2.score += pTank->level * 100;

    //Activate a score label
    activateScoreLabel(pTank);
}


bool activateScoreLabel(Tank *pTank)
{
    int i = 0;

    //find a score label that is not yet activated
    for(i = 0; i < MAX_SCORE_LABELS; i++)
    {
        if(!isTimerUp(&scoreLabelArray[i].timer))
            continue;

        break;
    }

    if(i == MAX_SCORE_LABELS)
    {
        printf("ERROR! NO DISABLED SCORE LABEL FOUND! FIX YOUR DAMN LOGIC!\n");
        return false;
    }

    //initialize the label found
    scoreLabelArray[i].rect.x = pTank->rect.x;
    scoreLabelArray[i].rect.y = pTank->rect.y;

    switch(pTank->level)
    {
        case 1:
            scoreLabelArray[i].pTex = rsmgrGetTexture(TEX_ID_SCORE_100);
            break;


        case 2:
            scoreLabelArray[i].pTex = rsmgrGetTexture(TEX_ID_SCORE_200);
            break;

        case 3:
            scoreLabelArray[i].pTex = rsmgrGetTexture(TEX_ID_SCORE_300);
            break;

        case 4:
            scoreLabelArray[i].pTex = rsmgrGetTexture(TEX_ID_SCORE_400);
            break;

        case 5:
            scoreLabelArray[i].pTex = rsmgrGetTexture(TEX_ID_SCORE_500);
            break;

        default:
            printf("THIS IS OS FUCKED UP! FIX YOUR LOGIC %s, %d\n", __FUNCTION__, __LINE__);
    }

    setTimer(&scoreLabelArray[i].timer, SCORE_LABEL_INTERVAL_MSEC);
    return true;
}

void renderScoreLabelArray(void)
{
    for(int i = 0; i < MAX_SCORE_LABELS; i++)
    {
        if(isTimerUp(&scoreLabelArray[i].timer))
            continue;

        SDL_RenderCopy(cfg.pRen, scoreLabelArray[i].pTex, NULL, &scoreLabelArray[i].rect);
    }
}

void initScoreLabelArray(void)
{
    memset(scoreLabelArray, 0, sizeof(scoreLabelArray));
    for(int i = 0; i < MAX_SCORE_LABELS; i++)
    {
        SDL_QueryTexture(rsmgrGetTexture(TEX_ID_SCORE_100), NULL, NULL, &scoreLabelArray[i].rect.w, &scoreLabelArray[i].rect.h);
    }
}


void initBonus(void)
{
    bonus.rect.w = 64;
    bonus.rect.h = 64;
    bonus.rect.x = 664;
    bonus.rect.y = 564;
    bonus.pTex = rsmgrGetTexture(TEX_ID_BONUS_STAR); /*NULL;*/
    bonus.pHandlerFunc = NULL;
/*
    bonusHandlerArray[0] = handleBonusStar;    
    bonusHandlerArray[1] = handleBonusTank;    
    bonusHandlerArray[2] = handleBonusShovel;    
    bonusHandlerArray[3] = handleBonusShip;    
    bonusHandlerArray[4] = handleBonusHelmet;    
    bonusHandlerArray[5] = handleBonusClock;    
    bonusHandlerArray[6] = handleBonusBomb;    
    bonusHandlerArray[7] = handleBonusGun;    

*/
    setTimer(&bonus.lifetimeTimer, BONUS_LIFETIME_INTERVAL_MS);
}
/*
void handleBonusStar(Tank *pTank)
{
    setTankLevel(pTank, pTank->level+1);
}
*/

/*
void handleBonusTank(Tank *pTank)
{
    switch(pTank->tankId)
    {
        case TANK_ID_PLAYER1:
            p1.lives++;
            break;

        case TANK_ID_PLAYER2:
            p2.lives++;
            break;

        default:
            break;
    }
}
*/

/* TODO: Implement this */
/*
void handleBonusHelmet(Tank *pTank) {}

void handleBonusGun(Tank *pTank)
{
    setTankLevel(pTank, 4);
}

void handleBonusBomb(Tank *pTank)
{
    for(int i = 2; i < MAX_TANKS; i++)
    {
        tankArray[i].enabled = false;
        activateScoreLabel(pTank);
    }
}
*/

/*TODO: IMPLEMENT THIS */
//void handleBonusClock(Tank *pTank)

/*TODO: IMPLEMENT THIS */
/*
void handleBonusShovel(Tank *pTank)

void handleBonusShip(Tank *pTank)
{
    pTank->hasBoat = true;
}
*/

void renderBonus(void)
{
    if(!isTimerUp(&bonus.lifetimeTimer))
    {
        if(isTimerUp(&bonus.blinkTimer))
        {
            setTimer(&bonus.blinkTimer, BONUS_BLINK_INTERVAL_MS);
            bonus.isVisible = !bonus.isVisible;
        }

        if(bonus.isVisible)
            SDL_RenderCopy(cfg.pRen, bonus.pTex, NULL, &bonus.rect);

    }
}

void pre_runPlayState(void)
{
    //build the terrain
    terrainBuilders[cfg.Level - 1]();

    //init the tank objects
    initTankArray();
 
    initBullets();

    //Init the bonus structure and the bonus handler array
    initBonus();

    cfg.enemiesLeft = 20;

	Mix_PlayMusic(rsmgrGetMusic(MUSIC_ID_IDLE), -1);

	//Last thing to do
	fsm.states[FSM_PLAY_STATE].run = runPlayState;
}

void goToGameOverState(void)
{
	Mix_HaltMusic();
	fsm.states[FSM_PLAY_STATE].run = pre_runPlayState;
	fsm.currentState = FSM_GAMEOVER_STATE;
}
