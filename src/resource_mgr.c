#include <stdio.h> 
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "global_defs.h"
#include "resource_mgr.h"

extern Context ctx;
static ResourceMgr rsmgr;

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
    pTextTex = SDL_CreateTextureFromSurface(ctx.pRen, pTextSurface);
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
    pTex = SDL_CreateTextureFromSurface(ctx.pRen, pSprite);
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

static bool loadMusic(int id, char* path)
{
    Mix_Music* mus = Mix_LoadMUS(path);

    if(!mus)
    {
        printf("FAILED TO LOAD MUSIC: %s\n", Mix_GetError());
        return false;
    }

    rsmgr.musicTable[id] = mus;
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
Mix_Music *rsmgrGetMusic(int musId)
{
    return rsmgr.musicTable[musId];
}

Mix_Chunk *rsmgrGetChunk(int chunkId)
{
    return rsmgr.chunkTable[chunkId];
}

SDL_Texture *rsmgrGetTexture(int texId)
{
    return rsmgr.texTable[texId];
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

    if(!loadTexture(TEX_ID_SCORE_500, SPRITE_PATH"/500.png"))
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

    /* Load fonts */
    if(!font2Tex(TEX_ID_PLAY_FONT,20, (SDL_Color){0, 0, 0, 255}))
            return false;

    if(!font2Tex(TEX_ID_GAMEOVER_FONT, 120, (SDL_Color){135, 0, 0, 255}))
            return false;

    /* Load music */
    if(!loadMusic(MUSIC_ID_IDLE, AUDIO_PATH"/idle.wav"))
        return false;

    /* Load audio chuncks */
    if(!loadChunk(CHUNK_ID_FIRE, AUDIO_PATH"/fire.wav"))
        return false;

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

    //Clean the music
    for(int i = 0; i < MAX_MUSIC_RESOURCES; i++)
    {
        if(rsmgr.musicTable[i])
            Mix_FreeMusic(rsmgr.musicTable[i]);
    }

    //Clean the audio chunks
    for(int i = 0; i < MAX_CHUNK_RESOURCES; i++)
    {
        if(rsmgr.chunkTable[i])
            Mix_FreeChunk(rsmgr.chunkTable[i]);
    }
}
