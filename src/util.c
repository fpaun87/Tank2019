#include <stdarg.h>
#include <SDL2/SDL.h>
#include "global_defs.h"
#include "resource_mgr.h"

extern Context ctx;

/* A function that renders an arbitrary text. The size 
 * and color of the text is determined by the 'texId' 
 * parameter. The 'fmt' parameter is the same as the one
 * from printf.
 */
void printfg(int texId, int x, int y, char *fmt, ...)
{
    static char outBuf[256];
    SDL_Texture *text = NULL;
    SDL_Rect totalTextRect = {0, 0, 0, 0};
    SDL_Rect srcRect = {0, 0, 0, 0};
    SDL_Rect dstRect = {0, 0, 0, 0};
    va_list ap;
    int glyphWidth = 0;
    va_start(ap, fmt);
    int str_size = vsprintf(outBuf, fmt, ap);
    if(str_size < 0)
        printf("There's nothing to print! %s: %d\n", __FUNCTION__, __LINE__);
    
    //Get the size of the reference font texture    
    text = rsmgrGetTexture(texId);
    SDL_QueryTexture(text, NULL, NULL, &totalTextRect.w, &totalTextRect.h);
    glyphWidth = totalTextRect.w/36; 
    srcRect.w = glyphWidth;
    srcRect.h = totalTextRect.h;
    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = glyphWidth;
    dstRect.h = totalTextRect.h;

    for(int i = 0; i < str_size; i++)
    {
        if(outBuf[i] >= 48 && outBuf[i] <= 57)
            srcRect.x = ((int)outBuf[i] - 48)*glyphWidth;
        else
            srcRect.x = ((int)outBuf[i] - 65 + 10)*glyphWidth;

        SDL_RenderCopy(ctx.pRen, text, &srcRect, &dstRect); 
        dstRect.x += glyphWidth;
    }
    va_end(ap);
}
