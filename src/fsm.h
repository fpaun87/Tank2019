#ifndef FSM_H
#define FSM_H

#define MAX_FSM_STATES 5

enum FSMStateId {
	FSM_MENU_STATE,
	FSM_LEVEL_STATE,
	FSM_PLAY_STATE,
	FSM_PAUSE_STATE,
	FSM_GAMEOVER_STATE,
	FSM_SCORE_STATE };

typedef void (*FSM_STATE_FUNC)(void);

typedef struct FSMState{
    FSM_STATE_FUNC run;
}FSMState;

typedef struct FSM{
    FSMState states[MAX_FSM_STATES];
    enum FSMStateId currentState;
}FSM;

#endif //FSM_H
