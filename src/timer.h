#ifndef TIMER_H
#define TIMER_H

#include <SDL2/SDL_timer.h>

/* A simple timer using SDL_GetTicks
 * All intervals are in ms
 */
typedef struct Timer{
    uint32_t interval;
    uint32_t startTime;
	bool paused;
	uint32_t pauseStart;
	uint32_t pauseInterval;
}Timer;

void setTimer(Timer *p_pTimer, int p_nInterval);
bool isTimerUp(Timer *p_pTimer);
void pauseTimer(Timer *p_pTimer);
void resumeTimer(Timer *p_pTimer);

#endif //TIMER_H
