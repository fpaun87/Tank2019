#include <stdbool.h>
#include <stdint.h>
#include "timer.h"

void setTimer(Timer *p_pTimer, int p_nInterval)
{
	p_pTimer->startTime = SDL_GetTicks();
	p_pTimer->interval = p_nInterval;
	p_pTimer->paused = false;
	p_pTimer->pauseStart = 0;
	p_pTimer->pauseInterval = 0;
}

bool isTimerUp(Timer *p_pTimer)
{
	if(p_pTimer->paused)
		return false;

	return (((SDL_GetTicks() - p_pTimer->startTime - p_pTimer->pauseInterval) >= p_pTimer->interval) ? true : false);
}

void pauseTimer(Timer *p_pTimer)
{
	//No need to pause an already paused timer
	if(p_pTimer->paused) return;

	p_pTimer->pauseStart = SDL_GetTicks();
	p_pTimer->paused = true;
}

void resumeTimer(Timer *p_pTimer)
{
	//Don't resume a ticking timer
	if(!p_pTimer->paused) return;

	p_pTimer->pauseInterval = (SDL_GetTicks() - p_pTimer->pauseStart);
	p_pTimer->paused = false;
}

