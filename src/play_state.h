#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include "global_defs.h"
#include "timer.h"

#define MAX_BULLETS 40
#define MAX_TANKS 6

#define DEFAULT_TANK_SPEED 2u //pixels per update step
#define DEFAULT_FIRE_INTERVAL 1000 //MS
#define DEFAULT_SPAWN_STATE_DURATION 4000 //MS
#define DEFAULT_BLOCKED_STATE_DURATION 5000 //MS
#define DEFAULT_IMMUNE_STATE_DURATION 10000 //MS
#define DEFAULT_BULLET_SPEED 4u //pixels per update step
#define MAX_SCORE_LABELS 10
#define SCORE_LABEL_INTERVAL_MSEC 2000

/* Definitions for the Bonus data type */
#define BONUS_BLINK_INTERVAL_MS  200
#define BONUS_LIFETIME_INTERVAL_MS 7000
#define BONUS_COUNT 15


typedef enum BONUS_TYPE{
    BONUS_TYPE_STAR,
    BONUS_TYPE_TANK,
    BONUS_TYPE_HELMET,
    BONUS_TYPE_GUN,
    BONUS_TYPE_BOMB,
    BONUS_TYPE_CLOCK,
    BONUS_TYPE_SHOVEL,
    BONUS_TYPE_SHIP
} BONUS_TYPE;

enum DriverType {HUMAN_DRIVER, CPU_DRIVER};
enum TankId {TANKID_PLAYER1, TANKID_PLAYER2, TANKID_ENEMY};

typedef struct Bonus{
    Timer blinkTimer;
    Timer lifetimeTimer;
    SDL_Rect rect;
    SDL_Texture *pTex;
    bool isVisible;
    BONUS_TYPE type;
    bool enabled;
}Bonus;

/* Forward declaration */
typedef struct Tank Tank;

#define TANK_FSM_MAX_STATES 7
typedef void (*TankFSMFuncPtr)(Tank* pTank);
typedef struct TankState {
    SDL_Texture *pTex;
    Timer blinkTimer;
	Timer timer;
	TankFSMFuncPtr input;
	TankFSMFuncPtr run;
	TankFSMFuncPtr render;
}TankState;

typedef struct TankFSM {
	TankState states[TANK_FSM_MAX_STATES];
	int currentState;
}TankFSM;

typedef enum BONUS_TYPE BONUS_TYPE;

typedef struct Tank{
    SDL_Rect rect;
	SDL_Rect spawn_rect;
    float angle;
    uint32_t speed;
    bool canFire;
	Timer holdTimer;
    enum MoveEvent newMe;
    enum MoveEvent currMe;
    enum FireEvent fe;
    bool hasBoat;
	bool hasBonus;
	BONUS_TYPE bonusType;
	enum DriverType driver;
	enum TankId id;
    int level;
    int hp;
	TankFSM fsm;
}Tank;

enum TankFsmState {
	TANK_NORMAL_STATE, 
	TANK_DEAD_STATE, 
	TANK_INVALID_STATE, 
	TANK_SPAWN_STATE, 
	TANK_PRE_SPAWN_STATE,
	TANK_BLOCKED_STATE,
	TANK_IMMUNE_STATE
};

typedef struct Bullet{
    SDL_Rect rect;
    SDL_Texture *pTex;
    float angle;
    uint32_t speed;
    Tank *pOwner;	//the tank that released the bullet
    bool enabled;
}Bullet;


typedef struct ScoreLabel{
	bool active;
    Timer timer;
    SDL_Rect rect;
    SDL_Texture *pTex;
}ScoreLabel;

typedef void (*BonusHandlerFuncPtr)(Tank *pTank, Bonus *pBonus);
#endif //PLAY_STATE_H
