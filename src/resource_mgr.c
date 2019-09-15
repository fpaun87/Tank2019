#include <stdio.h> 
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "global_defs.h"
#include "resource_mgr.h"

extern Config cfg;
static ResourceMgr rsmgr;

struct Block {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    int32_t type;
} __attribute__((packed));

struct Block bmap[MAX_TERRAIN_TILES];

static bool font2Tex(int fontTexId, int size, SDL_Color color)
{
    SDL_Surface *pTextSurface = NULL;
    SDL_Texture *pTextTex = NULL; 
    TTF_Font *pFont = TTF_OpenFont(FONT_PATH"/PressStart2P.ttf", size);
    if(!pFont)
    {
        printf("Error loading font ! %s\n", TTF_GetError());
        return false;
    }

    //Let's render our text on the surface
    pTextSurface = TTF_RenderText_Solid(pFont, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", color); 
    if(!pTextSurface)
    {
        printf("Could not render text from font! %s\n", TTF_GetError());
        TTF_CloseFont(pFont);
        return false;
    }

    //We don't need the font
    TTF_CloseFont(pFont);

    //Create a texture from the previous surface
    pTextTex = SDL_CreateTextureFromSurface(cfg.pRen, pTextSurface);
    if(!pTextTex)
    {
        printf("Could not create texture from surface %s, %s: %d\n", SDL_GetError(), __FUNCTION__, __LINE__);
        SDL_FreeSurface(pTextSurface);
        return false;
    }

    //We no longer need the surface
    SDL_FreeSurface(pTextSurface);
    rsmgr.texTable[fontTexId]= pTextTex;
    return true;
}

static bool loadTexture(int texId, const char * path)
{
    SDL_Surface *pSprite = NULL;
    SDL_Texture *pTex = NULL;

    pSprite = IMG_Load(path);
    if(!pSprite)
    {
        printf("Error loading sprite %s ! %s\n",path, SDL_GetError());
        return false;
    }

    //Now it's time to create a texture from this surface since we use the gpu to render
    pTex = SDL_CreateTextureFromSurface(cfg.pRen, pSprite);
    if(!pTex)
    {
        printf("Error creating texture from surface %s ! %s\n", path, SDL_GetError());
        return false;
    }
    rsmgr.texTable[texId]= pTex;
    //The surface is no longer needed
    SDL_FreeSurface(pSprite);   
 
    return true;
}

static bool loadChunk(int id, char* path)
{
    Mix_Chunk* chunk = Mix_LoadWAV(path);
    if(!chunk)
    {
        printf("FAILED TO LOAD AUDIO CHUNK: %s\n", Mix_GetError());
        return false;
    }
    rsmgr.chunkTable[id] = chunk;

    return true;
}

static bool loadMusic(char *path)
{
    Mix_Music *music = Mix_LoadMUS(path);
    if(!music)
    {
        printf("FAILED TO LOAD MUSIC: %s\n", Mix_GetError());
        return false;
    }
    rsmgr.pMusic = music;

    return true;
}

static bool loadMaps(void)
{
	char fileName[] = MAPS_PATH"/level00.map";
	int level = 1;
	int max = 50;
	FILE *file = NULL;
	TerrainTile *pTile = NULL;
	TerrainTile *pMap = NULL;

	for(level = 1; level <= max; level++)
	{
		sprintf(&fileName[strlen(MAPS_PATH) + 6], "%02d", level);
		sprintf(&fileName[strlen(MAPS_PATH) + 8], "%s", ".map");
		if(!(file = fopen(fileName, "rb")))
		{
			printf("%s, %d: Map: %s failed to open\n", __FUNCTION__, __LINE__, fileName);
			return false;
		}

		pMap = &rsmgr.allMaps[(level -1 )*MAX_TERRAIN_TILES];

		if(fread(bmap, MAX_TERRAIN_TILES*sizeof(struct Block), 1, file) != 1)
		{
			printf("%s, %d: Map: %s failed to load\n", __FUNCTION__, __LINE__, fileName);
			return false;
		}

		for(int i = 0; i < MAX_TERRAIN_TILES; i++)
		{
			pMap[i].rect.x = bmap[i].x;
			pMap[i].rect.y = bmap[i].y;
			pMap[i].rect.w = bmap[i].w;
			pMap[i].rect.h = bmap[i].h;
			pMap[i].type = bmap[i].type;
		}

		fclose(file);


		for(int i = 0; i < MAX_TERRAIN_TILES; i++)
		{
			pTile = &pMap[i];			
			pTile->rect.x += SCENE_TOP_LEFT_X;

			switch(pTile->type)
			{
				case TERRAIN_NONE:
					break;

				case TERRAIN_SHIELD:
					pTile->pTex =  rsmgrGetTexture(TEX_ID_SHIELD);
					break;

				case TERRAIN_FOREST:
					pTile->pTex =  rsmgrGetTexture(TEX_ID_FOREST);
					break;

				case TERRAIN_WATER:
					pTile->pTex =  rsmgrGetTexture(TEX_ID_WATER);
					break;

				case TERRAIN_ICE:
					pTile->pTex =  rsmgrGetTexture(TEX_ID_ICE);
					break;

				case TERRAIN_BRICK:
					pTile->pTex =  rsmgrGetTexture(TEX_ID_BRICK);
					break;

				case TERRAIN_EAGLE:
					pTile->pTex =  rsmgrGetTexture(TEX_ID_EAGLE);
					break;

				default:
					printf("%s : Wrong terrain type detected\n", __FUNCTION__);
					return false;
			}
		}
	}

	return true;
}


Mix_Chunk *rsmgrGetChunk(int chunkId)
{
    return rsmgr.chunkTable[chunkId];
}

Mix_Music *rsmgrGetMusic(void)
{
	return rsmgr.pMusic;
}

SDL_Texture *rsmgrGetTexture(int texId)
{
    return rsmgr.texTable[texId];
}

TerrainTile *rsmgrGetMap(int level)
{
	return &rsmgr.allMaps[(level - 1)*MAX_TERRAIN_TILES];
}

bool rsmgrInit(void)
{
    memset(&rsmgr, 0, sizeof(ResourceMgr));

    if(!loadTexture(TEX_ID_PLAYER1_LEVEL1, SPRITE_PATH"/p1l1_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER1_LEVEL2, SPRITE_PATH"/p1l2_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER1_LEVEL3, SPRITE_PATH"/p1l3_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER1_LEVEL4, SPRITE_PATH"/p1l4_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_BULLET, SPRITE_PATH"/bullet.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER2_LEVEL1, SPRITE_PATH"/p2l1_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER2_LEVEL2, SPRITE_PATH"/p2l2_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER2_LEVEL3, SPRITE_PATH"/p2l3_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_PLAYER2_LEVEL4, SPRITE_PATH"/p2l4_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_ENEMY_LEVEL1, SPRITE_PATH"/el1_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_ENEMY_LEVEL2, SPRITE_PATH"/el2_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_ENEMY_LEVEL3, SPRITE_PATH"/el3_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_ENEMY_LEVEL4, SPRITE_PATH"/el4_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_WATER, SPRITE_PATH"/water.png"))
        return false;

    if(!loadTexture(TEX_ID_FOREST, SPRITE_PATH"/forest.png"))
        return false;

    if(!loadTexture(TEX_ID_BRICK, SPRITE_PATH"/brick.png"))
        return false;

    if(!loadTexture(TEX_ID_SHIELD, SPRITE_PATH"/shield.png"))
        return false;

    if(!loadTexture(TEX_ID_ICE, SPRITE_PATH"/ice.png"))
        return false;

    if(!loadTexture(TEX_ID_EAGLE, SPRITE_PATH"/eagle.png"))
        return false;

    if(!loadTexture(TEX_ID_DEAD_EAGLE, SPRITE_PATH"/dead_eagle.png"))
        return false;

    if(!loadTexture(TEX_ID_SCORE_100, SPRITE_PATH"/100.png"))
        return false;

    if(!loadTexture(TEX_ID_SCORE_200, SPRITE_PATH"/200.png"))
        return false;

    if(!loadTexture(TEX_ID_SCORE_300, SPRITE_PATH"/300.png"))
        return false;

    if(!loadTexture(TEX_ID_SCORE_400, SPRITE_PATH"/400.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_STAR, SPRITE_PATH"/bonus_star.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_TANK, SPRITE_PATH"/bonus_tank.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_SHOVEL, SPRITE_PATH"/bonus_shovel.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_GUN, SPRITE_PATH"/bonus_gun.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_HELMET, SPRITE_PATH"/bonus_helmet.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_SHIP, SPRITE_PATH"/bonus_ship.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_BOMB, SPRITE_PATH"/bonus_bomb.png"))
        return false;

    if(!loadTexture(TEX_ID_BONUS_CLOCK, SPRITE_PATH"/bonus_clock.png"))
        return false;

    if(!loadTexture(TEX_ID_BOAT, SPRITE_PATH"/boat.png"))
        return false;

    if(!loadTexture(TEX_ID_MENU, SPRITE_PATH"/menu.png"))
        return false;

    if(!loadTexture(TEX_ID_HEALTH, SPRITE_PATH"/health32.png"))
        return false;

    if(!loadTexture(TEX_ID_COIN, SPRITE_PATH"/coin32.png"))
        return false;

    if(!loadTexture(TEX_ID_FLAG, SPRITE_PATH"/flag32.png"))
        return false;

    if(!loadTexture(TEX_ID_TANK_ICON, SPRITE_PATH"/tank_icon.png"))
        return false;

    if(!loadTexture(TEX_ID_ENEMY_TANK_SPAWN, SPRITE_PATH"/spawn.png"))
        return false;

    if(!loadTexture(TEX_ID_TANKL1_XRAY, SPRITE_PATH"/tankl1_xray.png"))
        return false;

    if(!loadTexture(TEX_ID_TANKL2_XRAY, SPRITE_PATH"/tankl2_xray.png"))
        return false;

    if(!loadTexture(TEX_ID_TANKL3_XRAY, SPRITE_PATH"/tankl3_xray.png"))
        return false;

    if(!loadTexture(TEX_ID_TANKL4_XRAY, SPRITE_PATH"/tankl4_xray.png"))
        return false;

    if(!loadTexture(TEX_ID_ENERGY_SHIELD, SPRITE_PATH"/energy_shield.png"))
        return false;

    /* Load fonts */
    if(!font2Tex(TEX_ID_PLAY_FONT,30, (SDL_Color){0, 40, 0, 255}))
            return false;

    if(!font2Tex(TEX_ID_GAMEOVER_FONT, 90, (SDL_Color){135, 0, 0, 255}))
            return false;

    if(!font2Tex(TEX_ID_MENU_FONT, 64, (SDL_Color){255, 255, 255, 255}))
            return false;

    if(!font2Tex(TEX_ID_LEVEL_FONT, 64, (SDL_Color){0, 0, 0, 255}))
            return false;

    if(!font2Tex(TEX_ID_PAUSE_FONT, 64, (SDL_Color){170, 170, 255, 255}))
            return false;


    /* Load audio chuncks */
    if(!loadChunk(CHUNK_ID_FIRE, AUDIO_PATH"/fire.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_BONUS_ACTIVE, AUDIO_PATH"/active_bonus.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_ANOTHER_LIFE, AUDIO_PATH"/another_life.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_DEFLECTED_BULLET, AUDIO_PATH"/deflected_bullet.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_EXPLOSION, AUDIO_PATH"/explosion.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_GAME_OVER, AUDIO_PATH"/game_over.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_GOT_BONUS, AUDIO_PATH"/got_bonus.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_STOPPED_BULLET, AUDIO_PATH"/stopped_bullet.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_TERRAIN_DESTRUCTION, AUDIO_PATH"/terrain_destruction.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_IDLE, AUDIO_PATH"/idle.wav"))
        return false;

    if(!loadChunk(CHUNK_ID_MOVE, AUDIO_PATH"/move.wav"))
        return false;

    if(!loadMusic(AUDIO_PATH"/intro.wav"))
        return false;

	/* Load the maps */
	if(!loadMaps())
		return false;

	if(SDL_GameControllerAddMappingsFromFile(RESOURCE_PATH"/gamecontrollerdb.txt") == -1)
	{
		printf("Gamepad mapping could not be loaded! %s\n", SDL_GetError());
		return false;
	}

    return true;
}

void rsmgrClose(void)
{
    //Clean the textures
    for(int i = 0; i < MAX_TEX_RESOURCES; i++)
    {
        if(rsmgr.texTable[i])
            SDL_DestroyTexture(rsmgr.texTable[i]);
    }


    //Clean the audio chunks
    for(int i = 0; i < MAX_CHUNK_RESOURCES; i++)
    {
        if(rsmgr.chunkTable[i])
            Mix_FreeChunk(rsmgr.chunkTable[i]);
    }

	Mix_FreeMusic(rsmgr.pMusic);

}
