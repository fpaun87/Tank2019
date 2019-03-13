#ifndef FSM_H
#define FSM_H

#define MAX_FSM_STATES 3

enum FSMStateId {FSM_MENU_STATE, FSM_PLAY_STATE, FSM_SCORE_STATE};

typedef void (*FSM_STATE_FUNC)(void);

typedef struct FSMState{
    enum FSMStateId id;
    FSM_STATE_FUNC handleInput;
    FSM_STATE_FUNC update;
    FSM_STATE_FUNC render;
}FSMState;

typedef struct FSM{
    FSMState states[MAX_FSM_STATES];
    enum FSMStateId currentState;
}FSM;

#endif //FSM_H
