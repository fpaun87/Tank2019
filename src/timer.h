#ifndef TIMER_H
#define TIMER_H

/* A simple timer using SDL_GetTicks
 * All intervals are in ms
 */
typedef struct Timer{
    uint32_t interval;
    uint32_t startTime;
}Timer;

/* Gets as parameters:
 *    - a pointer to a Timer struct
 *    - an int value
 */
#define TIMER_SET(mp_pTimer, mp_Interval) \
(mp_pTimer)->interval = (mp_Interval);    \
(mp_pTimer)->startTime = SDL_GetTicks();  \

//Gets as a parameter a pointer to a Timer struct;
/*
#define IS_TIMER_EXPIRED(mp_pTimer) \
(((SDL_GetTicks() - (mp_pTimer)->startTime) >= (mp_pTimer)->interval) ? true : false)
*/

#define IS_TIMER_TICKING(mp_pTimer) \
(((SDL_GetTicks() - (mp_pTimer)->startTime) < (mp_pTimer)->interval) ? true : false)
#endif //TIMER_H
