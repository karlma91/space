#include "stdlib.h"
#include "statesystem.h"

#define MAX_STATES 10
#define MAX_INNER_STATES 10

static int state_count = 0;

typedef struct systemstate sstate;
struct systemstate {
    sstate *prev;

    STATE_ID id;

    int inner_states;
    int current_inner_state;
    float time_alive;
    float time_in_inner_state;

    void (*inner_update[MAX_INNER_STATES])();
    void (*inner_draw[MAX_INNER_STATES])();

    void (*sdl_event)(SDL_Event *event);

    void (*on_enter)(void);
    void (*pre_update)(void);
    void (*post_update)(void);
    void (*draw)(void);
    void (*on_leave)(void);
    void (*destroy)(void);

    sstate *next;

};


static STATE_ID state_beeing_rendered = -1;

sstate states[MAX_STATES];

sstate *stack_bot;
sstate *stack_top;

void statesystem_init()
{
    int i;
    for(i = 0; i<state_count; i++){
        states[i].prev = NULL;
        states[i].next = NULL;
        states[i].current_inner_state = 0;
        states[i].time_alive = 0;
        states[i].time_in_inner_state = 0;
    }
}

STATE_ID statesystem_add_state(int inner_states,
        void (*on_enter)(),
        void (*pre_update)(),
        void (*post_update)(),
        void (*draw)(),
        void (*sdl_event)(),
        void (*on_leave)(),
        void (*destroy)())
{
	STATE_ID state = state_count++;
    states[state].id = state;

    states[state].inner_states = inner_states;
    if(inner_states > 0){
        //states[state].inner_update = malloc(sizeof(states[state].inner_update)*inner_states);
      //  states[state].inner_draw = malloc(sizeof(states[state].inner_draw)*inner_states);
    }
    states[state].on_enter = on_enter;
    states[state].pre_update = pre_update;
    states[state].post_update = post_update;
    states[state].sdl_event = sdl_event;
    states[state].draw = draw;
    states[state].on_leave = on_leave;
    states[state].destroy = destroy;

    return state;
}
void statesystem_add_inner_state(STATE_ID state, int inner_state, void (*update)(), void (*draw)())
{
    states[state].inner_update[inner_state] = update;
    states[state].inner_draw[inner_state] = draw;
}

void statesystem_set_inner_state(STATE_ID state, int inner_state)
{
    states[state].time_in_inner_state = 0;
    states[state].current_inner_state = inner_state;
}

void statesystem_push_state(STATE_ID state)
{
    states[state].time_alive = 0;
    stack_top->next = &(states[state]);
    stack_top->next->prev = stack_top;
    stack_top = stack_top->next;
}

void statesystem_pop_state()
{
    sstate *temp = stack_top;
    stack_top = stack_top->prev;
    temp->prev = NULL;
    stack_top->next = NULL;
}

void statesystem_set_state(STATE_ID state)
{
    int i;
    for(i = 0; i<state_count; i++){
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
	if (stack_top->pre_update) {
		stack_top->pre_update();
	}

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
    sstate *stack_temp = stack_bot;
    while(stack_temp){
    	state_beeing_rendered = stack_temp->id;
        stack_temp->draw();
        if(stack_temp->inner_states > 0 &&
                stack_temp->inner_draw[stack_temp->current_inner_state]){
            stack_temp->inner_draw[stack_temp->current_inner_state]();
        }
        stack_temp = stack_temp->next;

    }
    state_beeing_rendered = -1;
}

void statesystem_destroy()
{
    int i = 0;
    for(i=0; i<state_count; i++){
        if(states[i].destroy){
            states[i].destroy();
        }
        if(states[i].inner_states > 0){
           // free(states[i].inner_update);
          //  free(states[i].inner_draw);
        }
    }
}

void statesystem_push_event(SDL_Event *event)
{
	if (stack_top->sdl_event) {
		stack_top->sdl_event(event);
	}
}

STATE_ID statesystem_get_render_state()
{
	return state_beeing_rendered;
}
