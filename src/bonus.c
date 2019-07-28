#include <SDL2/SDL.h>
#include "play_state.h"
#include "resource_mgr.h"

/* Prototypes for the bonus functions */

void handleBonusStar(Tank *pTank, Bonus *pBonus);
void handleBonusTank(Tank *pTank, Bonus *pBonus);
void handleBonusHelmet(Tank *pTank, Bonus *pBonus);
void handleBonusGun(Tank *pTank, Bonus *pBonus);
void handleBonusBomb(Tank *pTank, Bonus *pBonus);
void handleBonusClock(Tank *pTank, Bonus *pBonus);
void handleBonusShovel(Tank *pTank, Bonus *pBonus);
void handleBonusShip(Tank *pTank, Bonus *pBonus);

extern Config cfg;
extern SDL_Texture *normalTexTbl[3][5];
extern SDL_Texture *deadTexTbl[5];
extern Tank tank_array[MAX_TANKS];
extern TerrainTile map[MAX_TERRAIN_TILES];

BonusHandlerFuncPtr bonusClbkArray[8]= {
     handleBonusStar,
     handleBonusTank,
     handleBonusHelmet,
     handleBonusGun,
     handleBonusBomb,
     handleBonusClock,
     handleBonusShovel,
     handleBonusShip
};


Bonus bonusArray[BONUS_COUNT] = {0};

int bonusTexTbl[8] = {
	TEX_ID_BONUS_STAR,
	TEX_ID_BONUS_TANK,
	TEX_ID_BONUS_HELMET,
	TEX_ID_BONUS_GUN,
	TEX_ID_BONUS_BOMB,
	TEX_ID_BONUS_CLOCK,
	TEX_ID_BONUS_SHOVEL,
	TEX_ID_BONUS_SHIP
};	

void setBonus(BONUS_TYPE type, Tank *pTank)
{
	Bonus *pBonus = NULL;
	int i = 0;

	for(i = 0; i < BONUS_COUNT; i++)
	{
		pBonus = &bonusArray[i];
		if(pBonus->enabled)
			continue;

		break;
	}

	pBonus->enabled = true;
	pBonus->isVisible = true;
	pBonus->rect.x = pTank->rect.x;
	pBonus->rect.y = pTank->rect.y;
	pBonus->type = type;
	pBonus->pTex = rsmgrGetTexture(bonusTexTbl[type]);
    setTimer(&pBonus->lifetimeTimer, BONUS_LIFETIME_INTERVAL_MS);
    setTimer(&pBonus->blinkTimer, BONUS_BLINK_INTERVAL_MS);
}

void initBonusArray(void)
{
	for(int i = 0; i < BONUS_COUNT; i++)
	{
		bonusArray[i].rect.w = 64;
		bonusArray[i].rect.h = 64;
		bonusArray[i].enabled = false;
		bonusArray[i].isVisible = false;
	}
}

void updateBonusArray(void)
{
	Bonus *pBonus = NULL;
	for(int i = 0; i < BONUS_COUNT; i++)
	{
		pBonus = &bonusArray[i];	
		if(pBonus->enabled && !isTimerUp(&pBonus->lifetimeTimer))
			continue;

		pBonus->enabled = false;
		pBonus->isVisible = false;
	}
}

void renderBonus(Bonus *pBonus)
{
	if(isTimerUp(&pBonus->blinkTimer))
	{
		setTimer(&pBonus->blinkTimer, BONUS_BLINK_INTERVAL_MS);
		pBonus->isVisible = !pBonus->isVisible;
	}

	if(pBonus->isVisible)
		SDL_RenderCopy(cfg.pRen, pBonus->pTex, NULL, &pBonus->rect);

}

void renderBonusArray(void)
{
	Bonus *pBonus = NULL;

	for(int i = 0; i < BONUS_COUNT; i++)
	{
		pBonus = &bonusArray[i];
		if(!pBonus->enabled)
			continue;

		renderBonus(pBonus);
	}
}

void handleBonusStar(Tank *pTank, Bonus *pBonus)
{

	pBonus->enabled = false;
	pBonus->isVisible = false;

	/* We return if we can't aquire the star, but don't 
	 * invalidate the star. Maybe others want it
	 */
	if((pTank->level + 1) >  4)
		return;

	pTank->hp = ++pTank->level;

	pTank->fsm.states[TANK_NORMAL_STATE].pTex = normalTexTbl[pTank->id][pTank->level];

}

void handleBonusTank(Tank *pTank, Bonus *pBonus)
{
	
	pBonus->enabled = false;
	pBonus->isVisible = false;

	if(pTank->driver == CPU_DRIVER)
		return;

	/* We return if we can't aquire the tank, but don't 
	 * invalidate the tank. Maybe the green tank wants it
	 */
	if(pTank->id == TANKID_PLAYER1)
		cfg.p1.lives++;

	if(pTank->id == TANKID_PLAYER2)
		cfg.p2.lives++;

}

void handleBonusHelmet(Tank *pTank, Bonus *pBonus)
{
	pBonus->enabled = false;
	pBonus->isVisible = false;
	pTank->fsm.currentState = TANK_IMMUNE_STATE;
	setTimer(&pTank->fsm.states[TANK_IMMUNE_STATE].timer, DEFAULT_IMMUNE_STATE_DURATION);
}

/* Raise the tank straight to level 4 */
void handleBonusGun(Tank *pTank, Bonus *pBonus)
{
	/* We return if we can't aquire the star, but don't 
	 * invalidate the star. Maybe others want it
	 */
	if((pTank->level + 1) >  4)
		return;

	pBonus->enabled = false;
	pBonus->isVisible = false;
	pTank->level = 4;
	pTank->hp = pTank->level;

	pTank->fsm.states[TANK_NORMAL_STATE].pTex = normalTexTbl[pTank->id][pTank->level];
	pTank->fsm.states[TANK_DEAD_STATE].pTex = deadTexTbl[pTank->level];
}

/* Put the oposite team in dead state */
void handleBonusBomb(Tank *pTank, Bonus *pBonus)
{
	enum DriverType type;
	Tank *pTank2 = NULL;
	Player *pp = NULL;
	pBonus->enabled = false;
	pBonus->isVisible = false;
	if(pTank->driver == HUMAN_DRIVER)
		type = CPU_DRIVER;
	else
		type = HUMAN_DRIVER;

	if(pTank->driver == HUMAN_DRIVER)
	{
		if(pTank->id == TANKID_PLAYER1)
			pp = &cfg.p1;
		else
			pp = &cfg.p2;
	}

	for(int i = 0; i < MAX_TANKS; i++)
	{
		pTank2 = &tank_array[i];

		/* Don't commit suicide */
		if(pTank == pTank2)
			continue;	

		/* Don't kill your team members */
		if(pTank->driver == pTank2->driver)
			continue;

		/* Only kill those who are in the normal state */
		if(pTank2->fsm.currentState != TANK_NORMAL_STATE)
			continue;

		if(pp)
			pp->score += pTank2->level*100;

		pTank2->fsm.currentState = TANK_DEAD_STATE;
	}
}

void handleBonusClock(Tank *pTank, Bonus *pBonus)
{
	Tank *pVictim = NULL;
	pBonus->enabled = false;
	pBonus->isVisible = false;

	for(int i = 0; i < MAX_TANKS; i++)
	{
		pVictim = &tank_array[i];
		if((pVictim->driver != pTank->driver) && (pVictim->fsm.currentState == TANK_NORMAL_STATE))
		{
			pVictim->fsm.currentState = TANK_BLOCKED_STATE;
			setTimer(&pVictim->fsm.states[TANK_BLOCKED_STATE].timer, DEFAULT_BLOCKED_STATE_DURATION);
		}
	}
	
}

void handleBonusShovel(Tank *pTank, Bonus *pBonus)
{
	int type = TERRAIN_BRICK;
	int texId = TEX_ID_BRICK;
	pBonus->enabled = false;
	pBonus->isVisible = false;

	if(pTank->driver == HUMAN_DRIVER)
	{
		type = TERRAIN_SHIELD;
		texId = TEX_ID_SHIELD; 
	}

	map[12*13+5].pTex = rsmgrGetTexture(texId);
    map[12*13+5].type = type;

	map[12*13+7].pTex = rsmgrGetTexture(texId);
    map[12*13+7].type = type;

	map[11*13+5].pTex = rsmgrGetTexture(texId);
    map[11*13+5].type = type;

	map[11*13+6].pTex = rsmgrGetTexture(texId);
    map[11*13+6].type = type;

	map[11*13+7].pTex = rsmgrGetTexture(texId);
    map[11*13+7].type = type;

}

void handleBonusShip(Tank *pTank, Bonus *pBonus)
{
	pBonus->enabled = false;
	pBonus->isVisible = false;
	pTank->hasBoat = true;
}

