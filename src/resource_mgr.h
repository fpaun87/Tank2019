#ifndef RESOURCE_MGR_H
#define RESOURCE_MGR_H

#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "play_state.h"

#define MAX_PATH_LEN 80
#define MAX_TEX_RESOURCES   49
#define MAX_CHUNK_RESOURCES 11
#define MAX_MUSIC_RESOURCES 1
#define RESOURCE_PATH "resources"
#define SPRITE_PATH   RESOURCE_PATH"/sprites"
#define FONT_PATH  RESOURCE_PATH"/fonts"
#define AUDIO_PATH  RESOURCE_PATH"/sound"
#define MAPS_PATH RESOURCE_PATH"/maps"

//Texture id's
#define TEX_ID_PLAYER1_LEVEL1   0
#define TEX_ID_PLAYER1_LEVEL2   1
#define TEX_ID_PLAYER1_LEVEL3   2
#define TEX_ID_PLAYER1_LEVEL4   3
#define TEX_ID_PLAYER2_LEVEL1   4
#define TEX_ID_PLAYER2_LEVEL2   5
#define TEX_ID_PLAYER2_LEVEL3   6
#define TEX_ID_PLAYER2_LEVEL4   7
#define TEX_ID_ENEMY_LEVEL1     8
#define TEX_ID_ENEMY_LEVEL2     9
#define TEX_ID_ENEMY_LEVEL3    10
#define TEX_ID_ENEMY_LEVEL4    11
#define TEX_ID_BULLET          12 
#define TEX_ID_PLAY_FONT       13
#define TEX_ID_FOREST          14
#define TEX_ID_WATER           15
#define TEX_ID_ICE             16
#define TEX_ID_BRICK           17
#define TEX_ID_SHIELD          18
#define TEX_ID_EAGLE           19
#define TEX_ID_DEAD_EAGLE      20
#define TEX_ID_GAMEOVER_FONT   21
#define TEX_ID_SCORE_100       22
#define TEX_ID_SCORE_200       23
#define TEX_ID_SCORE_300       24
#define TEX_ID_SCORE_400       25
#define TEX_ID_BOAT		       26
#define TEX_ID_BONUS_STAR      27
#define TEX_ID_BONUS_TANK      28
#define TEX_ID_BONUS_SHOVEL    29
#define TEX_ID_BONUS_SHIP      30
#define TEX_ID_BONUS_HELMET    31
#define TEX_ID_BONUS_CLOCK     32
#define TEX_ID_BONUS_BOMB      33
#define TEX_ID_BONUS_GUN       34
#define TEX_ID_MENU            35
#define TEX_ID_MENU_FONT       36
#define TEX_ID_LEVEL_FONT      37
#define TEX_ID_PAUSE_FONT      38
#define TEX_ID_HEALTH		   39
#define TEX_ID_COIN			   40
#define TEX_ID_FLAG			   41
#define TEX_ID_TANK_ICON	   42
#define TEX_ID_ENEMY_TANK_SPAWN 43
#define TEX_ID_TANKL1_XRAY     44
#define TEX_ID_TANKL2_XRAY     45
#define TEX_ID_TANKL3_XRAY     46
#define TEX_ID_TANKL4_XRAY     47
#define TEX_ID_ENERGY_SHIELD   48


//Audio Chunk IDs
#define CHUNK_ID_FIRE           		0
#define CHUNK_ID_BONUS_ACTIVE   		1
#define CHUNK_ID_ANOTHER_LIFE   		2
#define CHUNK_ID_DEFLECTED_BULLET 		3
#define CHUNK_ID_EXPLOSION				4
#define CHUNK_ID_GAME_OVER				5
#define CHUNK_ID_STOPPED_BULLET			6
#define CHUNK_ID_TERRAIN_DESTRUCTION 	7
#define CHUNK_ID_GOT_BONUS				8
#define CHUNK_ID_IDLE					9
#define CHUNK_ID_MOVE					10


#define MAX_TERRAIN_TILES 169

enum TerrainType {
    TERRAIN_NONE,
    TERRAIN_SHIELD,
    TERRAIN_FOREST,
    TERRAIN_WATER,
    TERRAIN_ICE,
    TERRAIN_BRICK,
    TERRAIN_EAGLE
};

typedef struct TerrainTile{
    SDL_Rect rect;
    SDL_Texture *pTex;
    enum TerrainType type;
}TerrainTile;

typedef struct ResourceMgr{
    SDL_Texture *texTable[MAX_TEX_RESOURCES];
    Mix_Chunk *chunkTable[MAX_CHUNK_RESOURCES];
	Mix_Music *pMusic;
	TerrainTile allMaps[50 * MAX_TERRAIN_TILES];
}ResourceMgr;


SDL_Texture *rsmgrGetTexture(int texId);
Mix_Chunk *rsmgrGetChunk(int chunkId);
Mix_Music *rsmgrGetMusic(void);
TerrainTile *rsmgrGetMap(int level);
bool rsmgrInit(void);
void rsmgrClose(void);

#endif //RESOURCE_MGR_H
