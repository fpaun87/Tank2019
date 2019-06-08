#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include "global_defs.h"
#include "timer.h"

#define MAX_BULLETS 20
#define MAX_TANKS 6
#define MAX_TERRAIN_TILES 169

#define DEFAULT_TANK_SPEED 4u //pixels per update step
#define DEFAULT_FIRE_INTERVAL 800 //MS
#define DEFAULT_SPAWN_STATE_DURATION 4000 //MS
#define DEFAULT_BULLET_SPEED 6u //pixels per update step
#define MAX_SCORE_LABELS 10
#define SCORE_LABEL_INTERVAL_MSEC 2000


enum TerrainType {TERRAIN_NONE, TERRAIN_SHIELD, TERRAIN_FOREST, TERRAIN_WATER, TERRAIN_ICE, TERRAIN_BRICK, TERRAIN_EAGLE};

enum DriverType {HUMAN_DRIVER, CPU_DRIVER};
enum TankId {TANKID_PLAYER1, TANKID_PLAYER2, TANKID_ENEMY};

/* Forward declaration */
typedef struct Tank Tank;

#define TANK_FSM_MAX_STATES 5
typedef void (*TankFSMFuncPtr)(Tank* pTank);
typedef struct TankState {
    SDL_Texture *pTex;
	TankFSMFuncPtr input;
	TankFSMFuncPtr run;
	TankFSMFuncPtr render;
}TankState;

typedef struct TankFSM {
	TankState states[TANK_FSM_MAX_STATES];
	int currentState;
}TankFSM;

typedef struct Tank{
    SDL_Rect rect;
	SDL_Rect spawn_rect;
    float angle;
    uint32_t speed;
    bool canFire;
    Timer timer1; 
	Timer timer2;
	Timer holdTimer;
    enum MoveEvent newMe;
    enum MoveEvent currMe;
    enum FireEvent fe;
    bool hasBoat;
	enum DriverType driver;
	enum TankId id;
    int level;
    int hp;
	TankFSM fsm;
}Tank;

enum TankFsmState {TANK_NORMAL_STATE, TANK_DEAD_STATE, TANK_INVALID_STATE, TANK_SPAWN_STATE, TANK_PRE_SPAWN_STATE};

typedef struct Bullet{
    SDL_Rect rect;
    SDL_Texture *pTex;
    float angle;
    uint32_t speed;
    Tank *pOwner;	//the tank that released the bullet
    bool enabled;
}Bullet;

typedef struct TerrainTile{
    SDL_Rect rect;
    SDL_Texture *pTex;
    enum TerrainType type;
}TerrainTile;

typedef struct ScoreLabel{
	bool active;
    Timer timer;
    SDL_Rect rect;
    SDL_Texture *pTex;
}ScoreLabel;

/* Definitions for the Bonus data type */
#define BONUS_BLINK_INTERVAL_MS  250
#define BONUS_LIFETIME_INTERVAL_MS 10000
typedef void (*BonusHandlerFuncPtr)(Tank *pTank);

typedef struct Bonus{
    Timer blinkTimer;
    Timer lifetimeTimer;
    SDL_Rect rect;
    SDL_Texture *pTex;
    BonusHandlerFuncPtr pHandlerFunc;
    bool isVisible;
}Bonus;

void updateTanks(void);
SDL_Rect* moveTank(Tank *pTank);
void fireTank(Tank *pTank);
void initTank(Tank *pTank, int level, int x, int y, float angle,
			 enum DriverType type, enum TankId id);
void initTankArray(void);
void initBullets(void);
void updateBullets(void);
void renderTanks(void);
void renderBullets(void);
bool isInScene(SDL_Rect* pRect);
bool initTerrain(void);
bool isInvalidMapLocation(int x, int y);
void drawTerrain(void);
void updatePlayState(void);
bool initPlayState(void);
void handleInputPlayState(void);
void renderPlayState(void);
void tankReadKeyboard(Tank* pTank);
void tankReadGamepad(Tank* pTank);
void renderPlayState(void);
void handleGameOver(void);
void renderGameOver(void);
#endif //PLAY_STATE_H
