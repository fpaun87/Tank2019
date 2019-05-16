#ifndef FSM_H
#define FSM_H

#define MAX_FSM_STATES 5

typedef void (*FSM_STATE_FUNC)(void);

typedef struct FSMState{
    FSM_STATE_FUNC run;
}FSMState;

typedef struct FSM{
    FSMState states[MAX_FSM_STATES];
    int currentState;
}FSM;

#endif //FSM_H
