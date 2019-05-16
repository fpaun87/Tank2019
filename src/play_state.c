#include "play_state.h"
#include "global_defs.h"
#include "fsm.h"
#include "util.h"
#include "resource_mgr.h"

extern FSM fsm;
extern Config cfg;
extern bool quit;

void runPlayState(void);
void setTankLevel(Tank* pTank, int level);
int handleBulletTankCollision(Bullet *pBullet);
int handleBulletTerrainCollision(Bullet *pBullet);
void playerTankHitByEnemyBullet(Tank *pTank);
void enemyTankHitByPlayerBullet(Tank *pAttacker, Tank *pVictim);
bool activateScoreLabel(Tank *pTank);
void renderScoreLabelArray(void);
void initScoreLabelArray(void);
void initTankIconArray(void);
void renderTankIconArray(void);
void pre_runPlayState(void);
void goToGameOverState(void);
void drawForest(void);
void updateTanks(void);
bool tankTerrainCollision(Tank* pTank, SDL_Rect *pRect);

/* Terrain building functions */
void buildTerrain1(void);

/*
 * The array of function pointers that point to
 * functions which build the terrain for each level
 */
typedef void (*BuildTerrainFuncPtr) (void);
BuildTerrainFuncPtr terrainBuilders[50] = { buildTerrain1, 0 };

Tank tank_array[MAX_TANKS];
SDL_Rect tankIconArray[20] = {0};
Bullet bullet_array[MAX_BULLETS];
SDL_Rect scene;
TerrainTile map[MAX_TERRAIN_TILES];
ScoreLabel scoreLabelArray[MAX_SCORE_LABELS];
BonusHandlerFuncPtr bonusHandlerArray[7];
Bonus bonus; 
Player p1 = {2,0};
Player p2 = {2,0};

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
        //outOfScene = false;
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

        //Even if we don't update the tank position, we always update the orientation
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

        pNewPos = moveTank(pTank);

        //don't go out of bounds
        if(!isInScene(pNewPos))
			continue;
            //outOfScene = true;

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
		//Let's change this into a function
		hitsTerrain = tankTerrainCollision(pTank, pNewPos);
                
        if(/*!outOfScene && */ !hitsTank && !hitsTerrain)
        {
            pTank->rect.x = pNewPos->x;
            pTank->rect.y = pNewPos->y;
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

	//Init the tank icon array
	initTankIconArray();

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
    movePlayer1Tank();
    movePlayer2Tank();
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
    //Render all the enabled bullets
    renderBullets();
    //Render the tanks
    renderTanks();
	//Render the forest
	drawForest();
    //Render the bonus
    renderBonus();
    //Render the score labels
    renderScoreLabelArray();
	//render the tank icons
	renderTankIconArray();
    //Render the text
    printfg(TEX_ID_PLAY_FONT, 1080, 500, "P I\n");
	icon.y = 530;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_HEALTH), NULL, &icon); 
    printfg(TEX_ID_PLAY_FONT, 1126, 530, "%02d", p1.lives);
	icon.y = 570;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_COIN), NULL, &icon); 
    printfg(TEX_ID_PLAY_FONT, 1126, 570, "%06d", p1.score);
	printfg(TEX_ID_PLAY_FONT, 1080, 630, "P II");
	icon.y = 660;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_HEALTH), NULL, &icon); 
	printfg(TEX_ID_PLAY_FONT, 1126, 660, "%02d", p2.lives);
	icon.y = 700;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_COIN), NULL, &icon); 
	printfg(TEX_ID_PLAY_FONT, 1126, 700, "%06d", p2.score);

	icon.y = 780;	
	SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_FLAG), NULL, &icon); 
    printfg(TEX_ID_PLAY_FONT, 1126, 780, "%02d", cfg.Level);

}

void movePlayer1Tank(void)
{
	Tank *pTank = &tank_array[0];
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

void movePlayer2Tank(void)
{
	Tank *pTank = &tank_array[1];
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
    if(!pTank->canFire)
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
	bullet_array[i].rect.x = pTank->rect.x + (tw - bw)/2;
	bullet_array[i].rect.y = pTank->rect.y + (tw - bw)/2;
    bullet_array[i].enabled = true;
    bullet_array[i].pOwner = pTank;
    pTank->fe = FE_NONE;

    //reset the fire timer after each successfull shot
    pTank->canFire = false;

    //Play the fire sound
    Mix_PlayChannel(-1, rsmgrGetChunk(CHUNK_ID_FIRE), 0);
}

bool initTank(Tank *pTank, int level, int x, int y, float angle,
			 enum DriverType driver, enum TankId id)
{
    memset(pTank, 0, sizeof(Tank));

	pTank->id = id;
    setTankLevel(pTank, level);
    pTank->rect.x = x;
    pTank->rect.y = y;
    pTank->speed = DEFAULT_TANK_SPEED; //pixels per frame
    pTank->angle = angle;
    pTank->newMe = ME_STOP;
    pTank->currMe = ME_STOP;
    pTank->fe = FE_NONE;
    pTank->enabled = true;
	pTank->hp = level;
	pTank->driver = driver;
	pTank->canFire = true;
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
    int hits= 0;

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
			bullet_array[i].pOwner->canFire = true;
            break;
        }

        //Check collisions with tanks
		hits += handleBulletTankCollision(&bullet_array[i]);

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
				bullet_array[j].pOwner->canFire = true;
                hits++;
            }
        }

        //Destroy bullets that hit terrain.
		hits += handleBulletTerrainCollision(&bullet_array[i]);

        if(hits)
            bullet_array[i].enabled = false;

        hits = 0;
            
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

    if(!initTank(&tank_array[0], 1, SCENE_TOP_LEFT_X + 4*64, SCENE_HEIGHT-64, 0.0f,
		HUMAN_DRIVER, TANKID_PLAYER1))
        return false;

    if(!initTank(&tank_array[1], 1, SCENE_TOP_LEFT_X + 8*64, SCENE_HEIGHT-64, 0.0f,
		HUMAN_DRIVER, TANKID_PLAYER2))
        return false;
    
    if(cfg.players == 1)
        tank_array[1].enabled = false; 
    
    //Init the first batch of enemy tanks
    initTank(&tank_array[2], 1, SCENE_TOP_LEFT_X, SCENE_TOP_LEFT_Y, 180.0f,
				CPU_DRIVER, TANKID_ENEMY);
    initTank(&tank_array[3], 1, SCENE_TOP_LEFT_X + 6*64, SCENE_TOP_LEFT_Y, 180.0f,
				CPU_DRIVER, TANKID_ENEMY);
    initTank(&tank_array[4], 1, SCENE_TOP_LEFT_X + 12*64, SCENE_TOP_LEFT_Y, 180.0f,
				CPU_DRIVER, TANKID_ENEMY);
    initTank(&tank_array[5], 1, SCENE_TOP_LEFT_X + 6*64, SCENE_TOP_LEFT_Y + 2*64, 180.0f,
				CPU_DRIVER, TANKID_ENEMY);

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
	i += 4;

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
	i += 5;

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
	i += 5;

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

/* TODO: THIS MAY NEED TO BE FIXED */
void setTankLevel(Tank* pTank, int level)
{
    int texId;
    pTank->level = level;

    if(pTank->id == TANKID_PLAYER1)
        texId = TEX_ID_PLAYER1_LEVEL1;
    
    if(pTank->id == TANKID_PLAYER2)
        texId = TEX_ID_PLAYER2_LEVEL1;

    if(pTank->id == TANKID_ENEMY)
        texId = TEX_ID_ENEMY_LEVEL1;

    pTank->pTex = rsmgrGetTexture(texId);
}

int handleBulletTankCollision(Bullet *pBullet)
{
	Tank *pTank = NULL; //for convenience
	bool hits = 0;
	for(int m  = 0; m < MAX_TANKS; m++)
	{
		pTank = &tank_array[m];

		/* We don't care about collision with disabled tanks */
		if(!pTank->enabled)
			continue;

		if(SDL_HasIntersection(&pBullet->rect, &pTank->rect) != SDL_TRUE)
			continue;

		//If you're hit by your own bullet, nothing happens
		if(pBullet->pOwner == pTank)
			continue;

		//If a player is hit by an enemy bullet
		if((pBullet->pOwner->driver == CPU_DRIVER) && (pTank->driver == HUMAN_DRIVER))
		{
			hits++;
			playerTankHitByEnemyBullet(pTank);
			continue;
		}

		//If an enemy is hit by a player's bullet
		if((pBullet->pOwner->driver == HUMAN_DRIVER) && (pTank->driver == CPU_DRIVER))
		{
			hits++;
			enemyTankHitByPlayerBullet(pBullet->pOwner, pTank);
			continue;
		}

		//If the 2 tanks from the same team hit eachother, nothing happens
		if(pBullet->pOwner->driver == pTank->driver)
		{
			hits++;
			pBullet->pOwner->canFire = true;
			continue;
		}

	}

	return hits;

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

    //this should be moved to a function that checks for the
	// victory conditions
    pTank->enabled = false;
	if(pTank->id == TANKID_PLAYER1)
		p1.lives --;

	if(pTank->id == TANKID_PLAYER2)
		p2.lives --;

}

void enemyTankHitByPlayerBullet(Tank *pAttacker, Tank *pVictim)
{
    pVictim->hp--;

    //If the victim is till alive
    if(pVictim->hp)
        return;

    pVictim->enabled = false;
    cfg.enemiesLeft--;

    if(pAttacker->id == TANKID_PLAYER1)
        p1.score += pVictim->level * 100;

    if(pAttacker->id == TANKID_PLAYER2)
        p2.score += pVictim->level * 100;

	pAttacker->canFire = true;
    //Activate a score label
    activateScoreLabel(pVictim);
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
        case TANKID_PLAYER1:
            p1.lives++;
            break;

        case TANKID_PLAYER2:
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

/* This determines if the tank hits the terrain */
bool tankTerrainCollision(Tank* pTank, SDL_Rect* pRect)
{
	/* Evaluate the line and column of 2 corners depending on the direction */
	int line1 = 0, col1 = 0, line2 = 0, col2 = 0;

	switch(pTank->currMe)
	{
		case ME_LEFT:
			line1 = pRect->y/64;	
			col1 = (pRect->x - SCENE_TOP_LEFT_X)/64;	
			line2 = (pRect->y + pRect->w - 1)/64;	
			col2 = col1;
			break;

		case ME_RIGHT:
			line1 = pRect->y/64;
			col1 = (pRect->x - SCENE_TOP_LEFT_X + pRect->w - 1)/64;	
			line2 = (pRect->y + pRect->w - 1)/64;
			col2 = col1;
			break;

		case ME_UP:
			line1 = pRect->y/64;	
			col1 = (pRect->x - SCENE_TOP_LEFT_X)/64;	
			line2 = line1;
			col2 = (pRect->x - SCENE_TOP_LEFT_X + pRect->w - 1)/64;	
			break;

		case ME_DOWN:
			line1 = (pRect->y + pRect->w - 1)/64;
			col1 = (pRect->x - SCENE_TOP_LEFT_X)/64;
			line2 = line1;
			col2 = (pRect->x - SCENE_TOP_LEFT_X + pRect->w - 1)/64;
			break;

		case ME_STOP:
			return false;
	}

	/* As soon as you detect a collision, return */
	if((map[line1 * 13 + col1].type == TERRAIN_BRICK)  ||
       (map[line1 * 13 + col1].type == TERRAIN_SHIELD) ||
       (map[line1 * 13 + col1].type == TERRAIN_EAGLE)  ||
	   ((map[line1 * 13 + col1].type == TERRAIN_WATER) && !pTank->hasBoat))
    {
            if(SDL_HasIntersection(pRect, &map[line1 * 13 + col1].rect) == SDL_TRUE)
            {
				pTank->currMe = ME_STOP;
				return true;
            }
    }

	if((map[line2 * 13 + col2].type == TERRAIN_BRICK)  ||
       (map[line2 * 13 + col2].type == TERRAIN_SHIELD) ||
       (map[line2 * 13 + col2].type == TERRAIN_EAGLE)  ||
	   ((map[line2*13 + col2].type == TERRAIN_WATER) && !pTank->hasBoat))
	{
            if(SDL_HasIntersection(pRect, &map[line2 * 13 + col2].rect) == SDL_TRUE)
            {
				pTank->currMe = ME_STOP;
				return true;
            }
	}

	return false;
}

int handleBulletTerrainCollision(Bullet *pBullet)
{
	bool hit = false;
	/* Evaluate the line and column of 2 corners depending on the direction */
	int line1 = 0, col1 = 0, line2 = 0, col2 = 0;

	switch((int)pBullet->angle)
	{
		case 270:
			line1 = pBullet->rect.y/64;	
			col1 = (pBullet->rect.x - SCENE_TOP_LEFT_X)/64;	
			line2 = (pBullet->rect.y + pBullet->rect.w - 1)/64;	
			col2 = col1;
			break;

		case 90:
			line1 = pBullet->rect.y/64;
			col1 = (pBullet->rect.x - SCENE_TOP_LEFT_X + pBullet->rect.w - 1)/64;	
			line2 = (pBullet->rect.y + pBullet->rect.w - 1)/64;
			col2 = col1;
			break;

		case 0:
			line1 = pBullet->rect.y/64;	
			col1 = (pBullet->rect.x - SCENE_TOP_LEFT_X)/64;	
			line2 = line1;
			col2 = (pBullet->rect.x - SCENE_TOP_LEFT_X + pBullet->rect.w - 1)/64;	
			break;

		case 180:
			line1 = (pBullet->rect.y + pBullet->rect.w - 1)/64;
			col1 = (pBullet->rect.x - SCENE_TOP_LEFT_X)/64;
			line2 = line1;
			col2 = (pBullet->rect.x - SCENE_TOP_LEFT_X + pBullet->rect.w - 1)/64;
			break;

		default:
			return false;
	}

	/* If you hit the eagle go to game over state */
	if((map[line1 * 13 + col1].type == TERRAIN_EAGLE) ||
       (map[line2 * 13 + col2].type == TERRAIN_EAGLE))
	{
		map[162].pTex = rsmgrGetTexture(TEX_ID_DEAD_EAGLE);
		pBullet->enabled = false;
		goToGameOverState();
		return true;
	}

	if((map[line1 * 13 + col1].type == TERRAIN_BRICK)  ||
       (map[line1 * 13 + col1].type == TERRAIN_SHIELD))
    {
		if(SDL_HasIntersection(&pBullet->rect, &map[line1 * 13 + col1].rect) == SDL_TRUE)
		{
			map[line1 * 13 + col1].pTex = NULL;
			map[line1 * 13 + col1].type = TERRAIN_NONE;
			hit = true;
			pBullet->pOwner->canFire = true;
		}
    }

	if((map[line2 * 13 + col2].type == TERRAIN_BRICK)  ||
       (map[line2 * 13 + col2].type == TERRAIN_SHIELD))
	{
		if(SDL_HasIntersection(&pBullet->rect, &map[line2 * 13 + col2].rect) == SDL_TRUE)
		{
			map[line2 * 13 + col2].pTex = NULL;
			map[line2 * 13 + col2].type = TERRAIN_NONE;
			pBullet->pOwner->canFire = true;
			hit = true;
		}
	}

	return hit;
}

void initTankIconArray(void)
{
	int startX = 1152;
	int startY = 100;
	int offset = 10;

	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			tankIconArray[i*2+j].x = startX + j*(32 + offset) ;
			tankIconArray[i*2+j].y = startY + i*32;
			tankIconArray[i*2+j].w = 32;
			tankIconArray[i*2+j].h = 32;
		}
	}
}

void renderTankIconArray(void)
{
	for(int i = cfg.enemiesLeft -1; i >= 0; i--)
		SDL_RenderCopy(cfg.pRen, rsmgrGetTexture(TEX_ID_TANK_ICON), NULL, &tankIconArray[i]);

}
