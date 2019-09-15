#include <stdio.h> 
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "../global_defs.h"
#include "../timer.h"
#include "../resource_mgr.h"
#include "../play_state.h"

bool appInit(void);
void appClose(void);
void appRender(void);
void appInput(void);
void appRun(void);
void drawTerrain(void);
void initTerrain(void);
bool loadTexture(int texId, const char * path);
void drawMenu(void);
void printHelp(void);

#define MAX_TEXTURES 19
#define GAP 20 //pixels
#define MENU_X_MIN SCENE_WIDTH + 120
#define MENU_X_MAX MENU_X_MIN + 3 * 64 + 2 * GAP
#define MENU_Y_MIN 150
#define MENU_Y_MAX MENU_Y_MIN + 6 * 64 + 5 * GAP

enum TexId  {
	WATER,
	FOREST,
	ICE,
	BRICK,
	BRICK_UP,
	BRICK_DOWN,
	BRICK_LEFT,
	BRICK_RIGHT,
	BRICK_Q3,
	BRICK_Q4,
	SHIELD,
	SHIELD_UP,
	SHIELD_DOWN,
	SHIELD_LEFT,
	SHIELD_RIGHT,
	SHIELD_Q3,
	SHIELD_Q4,
	//keep these the last
	EAGLE,
	CURSOR
};

typedef struct {
	SDL_Rect rect;
	int index;
	int left_limit;
	int right_limit;
	int up_limit;
	int down_limit;
}Cursor;

struct Block {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    int32_t type;
} __attribute__((packed));

typedef struct {
	TerrainTile tile;
	SDL_Rect rect;
}MenuTile;

FILE *pInFile = NULL;
FILE *pOutFile = NULL;

//The global app context
Config cfg;

bool quit = false;

SDL_Rect scene;

Cursor mapCursor = {{0, 0, 64, 64}, 0, 0, SCENE_WIDTH - 64, 0, SCENE_HEIGHT - 64};
Cursor menuCursor = {{MENU_X_MIN, MENU_Y_MIN, 64, 64}, 0, MENU_X_MIN, MENU_X_MAX, MENU_Y_MIN, MENU_Y_MAX};

struct Block bmap[MAX_TERRAIN_TILES];
TerrainTile map[MAX_TERRAIN_TILES];
MenuTile menu[MAX_TEXTURES];

SDL_Texture* texTable[MAX_TEXTURES];

int main(int argc, char *argv[])
{

	if((argc > 3) || (argc < 2))
	{
		printHelp();
		return -1;
	}


    if(!appInit())
        return -1;

	if(argc == 3) 
	{
		if(!(pOutFile = fopen(argv[2], "wb")))
		{
			printf("ERROR: Couldn't open output file %s\n", argv[2]);
			return -1;
		}

		if(!(pInFile = fopen(argv[1], "rb")))
		{
			printf("ERROR: Couldn't open input file %s\n", argv[1]);
			return -1;
		}
		if(fread(bmap, sizeof(bmap), 1, pInFile) != 1)
		{
			printf("Error reading the input file!\n");
			return -1;
		}


		//set the texture pointers
		for(int i=0; i < MAX_TERRAIN_TILES; i++)
		{
			map[i].rect.x = bmap[i].x;
			map[i].rect.y = bmap[i].y;
			map[i].rect.w = bmap[i].w;
			map[i].rect.h = bmap[i].h;
			map[i].type = bmap[i].type;

			switch(map[i].type)
			{
				case TERRAIN_BRICK:
					map[i].pTex = texTable[BRICK];
					break;

				case TERRAIN_SHIELD:
					map[i].pTex = texTable[SHIELD];
					break;

				case TERRAIN_WATER:
					map[i].pTex = texTable[WATER];
					break;

				case TERRAIN_ICE:
					map[i].pTex = texTable[ICE];
					break;

				case TERRAIN_EAGLE:
					map[i].pTex = texTable[EAGLE];
					break;

				case TERRAIN_FOREST:
					map[i].pTex = texTable[FOREST];
					break;

				default:
					map[i].pTex = NULL; 
					break;
			}
		}
	}
	else
	{
		if(!(pOutFile = fopen(argv[1], "wb")))
		{
			printf("ERROR: Couldn't open output file %s\n", argv[1]);
			return -1;
		}
	}

    /*
     * **** THE APPLICATION'S MAIN LOOP *****
     */
    uint32_t updateInterval = 1000u/(FPS);
    uint32_t tf = 0u; //ms
    uint32_t ti = SDL_GetTicks(); //ms

    while (!quit)
    {
        ti = SDL_GetTicks();
        appRun();
        tf = SDL_GetTicks();
        if((tf - ti) < updateInterval)
            SDL_Delay(tf - ti);
    }
    //Cleanup
    appClose();
	return 0;
}

bool appInit(void)
{
    memset(&cfg, 0, sizeof(Config));

    //Initialize the various SDL susbsystems 

    if(SDL_Init(SDL_INIT_VIDEO) <  0)
    {
        printf("SDL failed to properly initialize: %s\n", SDL_GetError());
        return false;
    }


    //Hide the mouse pointer
    SDL_ShowCursor(SDL_DISABLE);

    //Create a SDL window 
    cfg.pWin = SDL_CreateWindow("Tank2019 Map Editor", WND_TOP_LEFT_X, WND_TOP_LEFT_Y, WND_WIDTH, WND_HEIGHT, SDL_WINDOW_SHOWN);
    if(!cfg.pWin)
    {
        printf("SDL window could not be created! %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    //Create a renderer 
    cfg.pRen = SDL_CreateRenderer(cfg.pWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!cfg.pRen)
    {
        SDL_DestroyWindow(cfg.pWin);
		
        printf("The renderer could not be created! %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

	//init the scene
	memset(&scene, 0, sizeof(scene));
	scene.w = SCENE_WIDTH;
	scene.h = SCENE_HEIGHT;

	//Load the textures
    if(!loadTexture(WATER, SPRITE_PATH"/water.png"))
        return false;

    if(!loadTexture(FOREST, SPRITE_PATH"/forest.png"))
        return false;

    if(!loadTexture(BRICK, SPRITE_PATH"/brick.png"))
        return false;

    if(!loadTexture(BRICK_UP, SPRITE_PATH"/brick_up.png"))
        return false;

    if(!loadTexture(BRICK_DOWN, SPRITE_PATH"/brick_down.png"))
        return false;

    if(!loadTexture(BRICK_LEFT, SPRITE_PATH"/brick_left.png"))
        return false;

    if(!loadTexture(BRICK_RIGHT, SPRITE_PATH"/brick_right.png"))
        return false;

    if(!loadTexture(BRICK_Q3, SPRITE_PATH"/brick_q3.png"))
        return false;

    if(!loadTexture(BRICK_Q4, SPRITE_PATH"/brick_q4.png"))
        return false;

    if(!loadTexture(SHIELD, SPRITE_PATH"/shield.png"))
        return false;

    if(!loadTexture(SHIELD_UP, SPRITE_PATH"/shield_up.png"))
        return false;

    if(!loadTexture(SHIELD_DOWN, SPRITE_PATH"/shield_down.png"))
        return false;

    if(!loadTexture(SHIELD_LEFT, SPRITE_PATH"/shield_left.png"))
        return false;

    if(!loadTexture(SHIELD_RIGHT, SPRITE_PATH"/shield_right.png"))
        return false;

    if(!loadTexture(SHIELD_Q3, SPRITE_PATH"/shield_q3.png"))
        return false;

    if(!loadTexture(SHIELD_Q4, SPRITE_PATH"/shield_q4.png"))
        return false;

    if(!loadTexture(ICE, SPRITE_PATH"/ice.png"))
        return false;

    if(!loadTexture(EAGLE, SPRITE_PATH"/eagle.png"))
        return false;

    if(!loadTexture(CURSOR, SPRITE_PATH"/cursor.png"))
        return false;

	initTerrain();

	initMenu();

    return true;
}


void appClose(void)
{
	//close the files
	if(pInFile)
		fclose(pInFile);

	if(pOutFile)
		fclose(pOutFile);

	 //Clean the textures
    for(int i = 0; i < MAX_TEXTURES; i++)
    {
		SDL_DestroyTexture(texTable[i]);
    }


    SDL_DestroyRenderer(cfg.pRen);
    SDL_DestroyWindow(cfg.pWin);
    SDL_Quit();
}

void appRender(void)
{
    //First clear the renderer
    SDL_SetRenderDrawColor(cfg.pRen, 128,128,128,255);
    SDL_RenderClear(cfg.pRen);

    //Render the scene
    SDL_SetRenderDrawColor(cfg.pRen, 0,0,0,255);
    SDL_RenderFillRect(cfg.pRen, &scene);

	//Render the terrain
	drawTerrain();

	//Render the menu
	drawMenu();

	//Render the cursors
	SDL_RenderCopy(cfg.pRen, texTable[CURSOR], NULL, &mapCursor.rect);
	SDL_RenderCopy(cfg.pRen, texTable[CURSOR], NULL, &menuCursor.rect);

	SDL_RenderPresent(cfg.pRen);
}

void appRun(void)
{
	appInput();
	appRender();
}

void appInput(void)
{
    static SDL_Event event;
	static const Uint8* currentKeyStates = NULL;
    currentKeyStates = SDL_GetKeyboardState(NULL);
	Cursor *pCursor = &mapCursor;
	int gap = 0;
	int line = 13;
	int val = 0;

	if(currentKeyStates[SDL_SCANCODE_LSHIFT])
	{
		pCursor = &menuCursor;
		gap = GAP;
		line = 3;
	}

    //handle events on queue
    while(SDL_PollEvent(&event))
    {
        if((event.type == SDL_QUIT))
        {
            quit = true;
            return;
        }

        if(event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    quit = true;
                    break;

				case SDLK_RIGHT:
					val = pCursor->rect.x + 64 + gap;
					if(val <= pCursor->right_limit)
					{
						pCursor->rect.x = val;
						pCursor->index++;
					}
					break;

				case SDLK_LEFT:
					val = pCursor->rect.x - 64 - gap;
					if(val >= pCursor->left_limit)
					{
						pCursor->rect.x = val;
						pCursor->index--;
					}
					break;


				case SDLK_DOWN:
					val = pCursor->rect.y + 64 + gap;
					if(val <= pCursor->down_limit)
					{
						pCursor->rect.y = val;
						pCursor->index += line;
					}
					break;

				case SDLK_UP:
					val = pCursor->rect.y - 64 - gap;
					if(val >= pCursor->up_limit)
					{
						pCursor->rect.y = val;
						pCursor->index -= line;
					}
					break;

				case SDLK_SPACE:
					map[mapCursor.index].type = menu[menuCursor.index].tile.type;
					map[mapCursor.index].rect.w = menu[menuCursor.index].tile.rect.w;
					map[mapCursor.index].rect.h = menu[menuCursor.index].tile.rect.h;
					map[mapCursor.index].rect.x = mapCursor.rect.x + menu[menuCursor.index].tile.rect.x;
					map[mapCursor.index].rect.y = mapCursor.rect.y + menu[menuCursor.index].tile.rect.y;
					map[mapCursor.index].pTex = menu[menuCursor.index].tile.pTex;

					bmap[mapCursor.index].type = menu[menuCursor.index].tile.type;
					bmap[mapCursor.index].w = menu[menuCursor.index].tile.rect.w;
					bmap[mapCursor.index].h = menu[menuCursor.index].tile.rect.h;
					bmap[mapCursor.index].x = mapCursor.rect.x + menu[menuCursor.index].tile.rect.x;
					bmap[mapCursor.index].y = mapCursor.rect.y + menu[menuCursor.index].tile.rect.y;
					break;

				case SDLK_s:
					if(fwrite(bmap, sizeof(bmap), 1, pOutFile) == 1)
						printf("Map successfully written to disk!\n");

					break;

				case SDLK_r:
					initTerrain();
					break;

				case SDLK_x:
					map[mapCursor.index].type = TERRAIN_NONE;
					map[mapCursor.index].pTex = NULL;
					bmap[mapCursor.index].type = TERRAIN_NONE;
					break;

				default:
					return;

            }
        }
    }

}

void drawTerrain(void)
{
    static SDL_Rect rect = {0};
	int j = 0;

    for(int i = 0; i < MAX_TERRAIN_TILES; i++)
    {
		j = 0;
        rect.w = map[i].rect.w;
        rect.h = map[i].rect.h;

		//find the menu tile coresponding to this terrain type
		if((map[i].type != TERRAIN_NONE) && (map[i].type != TERRAIN_EAGLE))
		{
			while(menu[j].tile.pTex != map[i].pTex)
				j++;
		}

		rect.x = menu[j].tile.rect.x;
		rect.y = menu[j].tile.rect.y;
		
        SDL_RenderCopy(cfg.pRen, map[i].pTex, &rect, &map[i].rect);
    }
}

void initTerrain(void)
{
	int i = 0;
	memset(map, 0, sizeof(map));
	for(int y = 0; y < SCENE_HEIGHT; y += 64)
	{
		for(int x = 0; x < SCENE_WIDTH; x += 64)
		{
			map[i].rect.x = x;
			map[i].rect.y = y;
			map[i].rect.w = 64;
			map[i].rect.h = 64;
			map[i].type = TERRAIN_NONE;

			bmap[i].x = x;
			bmap[i].y = y;
			bmap[i].w = 64;
			bmap[i].h = 64;
			bmap[i].type = TERRAIN_NONE;

			i++;
		}
	}

	//put the eagle
	map[12*13 + 6].type = TERRAIN_EAGLE;
	map[12*13 + 6].pTex = texTable[EAGLE];

	//now surround the eagle with brick
	map[12*13 + 5].type = TERRAIN_BRICK;
	map[12*13 + 5].pTex = texTable[BRICK_RIGHT];
	map[12*13 + 5].rect.x = 5 * 64 + 32;
	map[12*13 + 5].rect.y = 12*64;
	map[12*13 + 5].rect.w = 32;
	map[12*13 + 5].rect.h = 64;

	map[12*13 + 7].type = TERRAIN_BRICK;
	map[12*13 + 7].pTex = texTable[BRICK_LEFT];
	map[12*13 + 7].rect.x = 7 * 64;
	map[12*13 + 7].rect.y = 12*64;
	map[12*13 + 7].rect.w = 32;
	map[12*13 + 7].rect.h = 64;


	map[11*13 + 6].type = TERRAIN_BRICK;
	map[11*13 + 6].pTex = texTable[BRICK_DOWN];
	map[11*13 + 6].rect.x = 6 * 64;
	map[11*13 + 6].rect.y = 11*64 + 32;
	map[11*13 + 6].rect.w = 64;
	map[11*13 + 6].rect.h = 32;

	map[11*13 + 5].type = TERRAIN_BRICK;
	map[11*13 + 5].pTex = texTable[BRICK_Q4];
	map[11*13 + 5].rect.x = 5 * 64 + 32;
	map[11*13 + 5].rect.y = 11*64 + 32;
	map[11*13 + 5].rect.w = 32;
	map[11*13 + 5].rect.h = 32;

	map[11*13 + 7].type = TERRAIN_BRICK;
	map[11*13 + 7].pTex = texTable[BRICK_Q3];
	map[11*13 + 7].rect.x = 7 * 64;
	map[11*13 + 7].rect.y = 11*64 + 32;;
	map[11*13 + 7].rect.w = 32;
	map[11*13 + 7].rect.h = 32;


	//put the eagle in the bmap
	bmap[12*13 + 6].type = TERRAIN_EAGLE;

	//now surround the eagle with brick in the bmap
	bmap[12*13 + 5].type = TERRAIN_BRICK;
	bmap[12*13 + 5].x = 5 * 64 + 32;
	bmap[12*13 + 5].y = 12*64;
	bmap[12*13 + 5].w = 32;
	bmap[12*13 + 5].h = 64;

	bmap[12*13 + 7].type = TERRAIN_BRICK;
	bmap[12*13 + 7].x = 7 * 64;
	bmap[12*13 + 7].y = 12*64;
	bmap[12*13 + 7].w = 32;
	bmap[12*13 + 7].h = 64;


	bmap[11*13 + 6].type = TERRAIN_BRICK;
	bmap[11*13 + 6].x = 6 * 64;
	bmap[11*13 + 6].y = 11*64 + 32;
	bmap[11*13 + 6].w = 64;
	bmap[11*13 + 6].h = 32;

	bmap[11*13 + 5].type = TERRAIN_BRICK;
	bmap[11*13 + 5].x = 5 * 64 + 32;
	bmap[11*13 + 5].y = 11*64 + 32;
	bmap[11*13 + 5].w = 32;
	bmap[11*13 + 5].h = 32;

	bmap[11*13 + 7].type = TERRAIN_BRICK;
	bmap[11*13 + 7].x = 7 * 64;
	bmap[11*13 + 7].y = 11*64 + 32;;
	bmap[11*13 + 7].w = 32;
	bmap[11*13 + 7].h = 32;

}

bool loadTexture(int texId, const char * path)
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

    texTable[texId]= pTex;
    //The surface is no longer needed
    SDL_FreeSurface(pSprite);

    return true;
}

void initMenu(void)
{
	int i = 0;
	memset(menu, 0, sizeof(menu));
	for(int y = MENU_Y_MIN; y < MENU_Y_MAX; y += (64 + GAP))
	{
		for(int x = MENU_X_MIN; x < MENU_X_MAX; x += (64 + GAP))
		{
			menu[i].rect.x = x;
			menu[i].rect.y = y;
			menu[i].rect.w = 64;
			menu[i].rect.h = 64;
			menu[i].tile.pTex = texTable[i];
			
			menu[i].tile.rect.w = 64;
			menu[i].tile.rect.h = 64;
			
			switch(i)
			{
				
				case WATER:
					menu[i].tile.type = TERRAIN_WATER;
					break;


				case FOREST:
					menu[i].tile.type = TERRAIN_FOREST;
					break;

				case ICE:
					menu[i].tile.type = TERRAIN_ICE;
					break;

				case BRICK:
					menu[i].tile.type = TERRAIN_BRICK;
					break;

				case BRICK_UP:
					menu[i].tile.type = TERRAIN_BRICK;
					menu[i].tile.rect.h = 32;
					break;

				case BRICK_DOWN:
					menu[i].tile.type = TERRAIN_BRICK;
					menu[i].tile.rect.h = 32;
					menu[i].tile.rect.y = 32;
					break;

				case BRICK_LEFT:
					menu[i].tile.type = TERRAIN_BRICK;
					menu[i].tile.rect.w = 32;
					break;

				case BRICK_RIGHT:
					menu[i].tile.type = TERRAIN_BRICK;
					menu[i].tile.rect.w = 32;
					menu[i].tile.rect.x = 32;
					break;

				case BRICK_Q3:
					menu[i].tile.type = TERRAIN_BRICK;
					menu[i].tile.rect.w = 32;
					menu[i].tile.rect.h = 32;
					menu[i].tile.rect.y = 32;
					break;

				case BRICK_Q4:
					menu[i].tile.type = TERRAIN_BRICK;
					menu[i].tile.rect.w = 32;
					menu[i].tile.rect.h = 32;
					menu[i].tile.rect.x = 32;
					menu[i].tile.rect.y = 32;
					break;

				case SHIELD:
					menu[i].tile.type = TERRAIN_SHIELD;
					break;

				case SHIELD_UP:
					menu[i].tile.type = TERRAIN_SHIELD;
					menu[i].tile.rect.h = 32;
					break;

				case SHIELD_DOWN:
					menu[i].tile.type = TERRAIN_SHIELD;
					menu[i].tile.rect.h = 32;
					menu[i].tile.rect.y = 32;
					break;

				case SHIELD_LEFT:
					menu[i].tile.type = TERRAIN_SHIELD;
					menu[i].tile.rect.w = 32;
					break;

				case SHIELD_RIGHT:
					menu[i].tile.type = TERRAIN_SHIELD;
					menu[i].tile.rect.w = 32;
					menu[i].tile.rect.x = 32;
					break;

				case SHIELD_Q3:
					menu[i].tile.type = TERRAIN_SHIELD;
					menu[i].tile.rect.w = 32;
					menu[i].tile.rect.h = 32;
					menu[i].tile.rect.y = 32;
					break;

				case SHIELD_Q4:
					menu[i].tile.type = TERRAIN_SHIELD;
					menu[i].tile.rect.w = 32;
					menu[i].tile.rect.h = 32;
					menu[i].tile.rect.x = 32;
					menu[i].tile.rect.y = 32;
					break;

				default:
					return;
			}
			i++;
		}
	}

}

void drawMenu(void)
{
    static SDL_Rect rect = {0};
    for(int i = 0; i < MAX_TEXTURES-2; i++)
    {

        rect.w = menu[i].rect.w;
        rect.h = menu[i].rect.h;
        SDL_RenderCopy(cfg.pRen, menu[i].tile.pTex, NULL, &menu[i].rect);
    }
}

void printHelp(void)
{
	printf("USAGE:\n");
	printf(" te  <OUTPUT_FILE>\n");
	printf(" te  <INPUT_FILE> <OUTPUT_FILE>\n");
	printf("\nACTIONS\n");
	printf("------------\n");
	printf("r - Reset everything\nx - Delete under cursor\ns - Save to the output file\n");
}
