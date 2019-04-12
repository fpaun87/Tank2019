#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include "global_defs.h"
#include "timer.h"

#define MAX_BULLETS 20
#define MAX_TANKS 6
#define MAX_TERRAIN_TILES 169

#define DEFAULT_TANK_SPEED 4u //pixels per update step
#define DEFAULT_BULLET_SPEED 8u //pixels per update step
#define DEFAULT_FIRE_HOLDOUT 500u //ms
#define MAX_SCORE_LABELS 5
#define SCORE_LABEL_INTERVAL_MSEC 2000

enum TerrainType {TERRAIN_NONE, TERRAIN_SHIELD, TERRAIN_FOREST, TERRAIN_WATER, TERRAIN_ICE, TERRAIN_BRICK, TERRAIN_EAGLE};
enum TankID {TANK_ID_PLAYER1, TANK_ID_PLAYER2, TANK_ID_ENEMY};

typedef struct Tank{
    SDL_Rect rect;
    SDL_Rect tempRect;
    SDL_Texture *pTex;
    float angle;
    uint32_t speed;
    uint32_t fireHoldout;
    Timer timer;
    Timer *pTimer;
    enum MoveEvent newMe;
    enum MoveEvent currMe;
    enum FireEvent fe;
    enum TankID id;
    bool enabled;
    bool hasBoat;
    int level;
    int hp;
}Tank;

typedef struct Bullet{
    SDL_Rect rect;
    SDL_Texture *pTex;
    float angle;
    uint32_t speed;
    enum TankID tankId;
    bool enabled;
}Bullet;

typedef struct TerrainTile{
    SDL_Rect rect;
    SDL_Texture *pTex;
    enum TerrainType type;
}TerrainTile;

typedef struct Player{
    int lives;
    int score;
    Tank* pTank;
}Player;

typedef struct ScoreLabel{
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
bool initTank(Tank *pTank, int texId, int x, int y, float angle, enum TankID id);
bool initTankArray(void);
bool initBullets(void);
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
void moveTankByGamepad(Tank *pTank);
void moveTankByKeyboard(Tank *pTank);
void renderPlayState(void);
void handleGameOver(void);
void renderGameOver(void);
#endif //PLAY_STATE_H
