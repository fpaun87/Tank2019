#ifndef RESOURCE_MGR_H
#define RESOURCE_MGR_H

#include <stdbool.h>
#include <SDL2/SDL_ttf.h>

#define MAX_PATH_LEN 80
#define MAX_TEX_RESOURCES 35
#define RESOURCE_PATH "/home/florin/projects/c/tank2018/resources/"
#define SPRITE_PATH   RESOURCE_PATH"/sprites"
#define FONT_PATH  RESOURCE_PATH"/fonts"

//Texture id's
#define TEX_ID_PLAYER1_LEVEL1 0
#define TEX_ID_PLAYER1_LEVEL2 1
#define TEX_ID_PLAYER1_LEVEL3 2
#define TEX_ID_PLAYER1_LEVEL4 3
#define TEX_ID_PLAYER2_LEVEL1 4
#define TEX_ID_PLAYER2_LEVEL2 5
#define TEX_ID_PLAYER2_LEVEL3 6
#define TEX_ID_PLAYER2_LEVEL4 7
#define TEX_ID_ENEMY_LEVEL1   8
#define TEX_ID_ENEMY_LEVEL2   9
#define TEX_ID_ENEMY_LEVEL3  10
#define TEX_ID_ENEMY_LEVEL4  11
#define TEX_ID_BULLET        12 
#define TEX_ID_PLAY_FONT     13
#define TEX_ID_FOREST        14
#define TEX_ID_WATER         15
#define TEX_ID_ICE           16
#define TEX_ID_BRICK         17
#define TEX_ID_SHIELD        18
#define TEX_ID_EAGLE         19
#define TEX_ID_DEAD_EAGLE    20
#define TEX_ID_GAMEOVER_FONT 21
#define TEX_ID_SCORE_100     22
#define TEX_ID_SCORE_200     23
#define TEX_ID_SCORE_300     24
#define TEX_ID_SCORE_400     25
#define TEX_ID_SCORE_500     26
#define TEX_ID_BONUS_STAR      27
#define TEX_ID_BONUS_TANK      28
#define TEX_ID_BONUS_SHOVEL    29
#define TEX_ID_BONUS_SHIP      30
#define TEX_ID_BONUS_HELMET    31
#define TEX_ID_BONUS_CLOCK     32
#define TEX_ID_BONUS_BOMB      33
#define TEX_ID_BONUS_GUN       34

typedef struct TexTableEntry{
    int id;
    SDL_Texture *pTex;
}TexTableEntry;

typedef struct ResourceMgr{
    TexTableEntry texTable[MAX_TEX_RESOURCES];
}ResourceMgr;


SDL_Texture *rsmgrGetTexture(int texId);
bool rsmgrInit(void);
void rsmgrClose(void);

#endif //RESOURCE_MGR_H
