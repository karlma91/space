#include "stdlib.h"
#include "statesystem.h"
#include "menu.h"
#include "space.h"
#include "gameover.h"
#include "levelselect.h"

typedef struct systemstate sstate;
struct systemstate {

    sstate *prev;

    int inner_states;
    int current_inner_state;
    float time_alive;
    float time_in_inner_state;

    void (*inner_update[10])();
    void (*inner_draw[10])();

    void (*on_enter)(void);
    void (*pre_update)(void);
    void (*post_update)(void);
    void (*draw)(void);
    void (*on_leave)(void);
    void (*destroy)(void);

    sstate *next;

};

sstate states[STATESYSTEM_COUNT];

sstate *stack_bot;
sstate *stack_top;


void statesystem_init()
{
    int i;
    for(i = 0; i<STATESYSTEM_COUNT; i++){
        states[i].prev = NULL;
        states[i].next = NULL;
        states[i].current_inner_state = 0;
        states[i].time_alive = 0;
        states[i].time_in_inner_state = 0;
    }

    menu_init();
    space_init();
    gameover_init();
    levelselect_init();

    statesystem_set_state(0);

}

void statesystem_init_state(int state, int inner_states,
        void (*on_enter)(),
        void (*pre_update)(),
        void (*post_update)(),
        void (*draw)(),
        void (*on_leave)(),
        void (*destroy)())
{

    states[state].inner_states = inner_states;
    if(inner_states > 0){
        //states[state].inner_update = malloc(sizeof(states[state].inner_update)*inner_states);
      //  states[state].inner_draw = malloc(sizeof(states[state].inner_draw)*inner_states);
    }
    states[state].on_enter = on_enter;
    states[state].pre_update = pre_update;
    states[state].post_update = post_update;
    states[state].draw = draw;
    states[state].on_leave = on_leave;
    states[state].destroy = destroy;
}
void statesystem_add_inner_state(int state, int inner_state, void (*update)(), void (*draw)())
{
    states[state].inner_update[inner_state] = update;
    states[state].inner_draw[inner_state] = draw;
}

void statesystem_set_inner_state(int state, int inner_state)
{
    states[state].time_in_inner_state = 0;
    states[state].current_inner_state = inner_state;
}

void statesystem_push_state(int state)
{
    states[state].time_alive = 0;
    stack_top->next = &(states[state]);
    stack_top->next->prev = stack_top;
    stack_top = stack_top->next;
}

void statesystem_pop_state(int state)
{
    sstate *temp = stack_top;
    stack_top = stack_top->prev;
    temp->prev = NULL;
    stack_top->next = NULL;
}

void statesystem_set_state(int state)
{
    int i;
    for(i = 0; i<STATESYSTEM_COUNT; i++){
        states[i].prev = NULL;
        states[i].next = NULL;
    }
    states[state].on_enter();
    states[state].time_alive = 0;
    stack_top = &(states[state]);
    stack_bot = &(states[state]);
}

void statesystem_update()
{
	stack_top->pre_update();
	if(stack_top->inner_states > 0 &&
			stack_top->inner_update[stack_top->current_inner_state]){
		stack_top->inner_update[stack_top->current_inner_state]();
	}
    if( stack_top->post_update){
        stack_top->post_update();
    }
}

void statesystem_draw()
{
	//TODO move all gl dependent code out of this file!
#if GLES2


#else
    sstate *stack_temp = stack_bot;
    while(stack_temp){
        glLoadIdentity();
        stack_temp->draw();
        if(stack_temp->inner_states > 0 &&
                stack_temp->inner_draw[stack_temp->current_inner_state]){
            stack_temp->inner_draw[stack_temp->current_inner_state]();
        }
        stack_temp = stack_temp->next;
    }
#endif
}

void statesystem_destroy()
{
    int i = 0;
    for(i=0; i<STATESYSTEM_COUNT; i++){
        if(states[i].destroy){
            states[i].destroy();
        }
        if(states[i].inner_states > 0){
           // free(states[i].inner_update);
          //  free(states[i].inner_draw);
        }
    }
}

