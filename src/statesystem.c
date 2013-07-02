#include "stdlib.h"
#include "statesystem.h"

#define MAX_INNER_STATES 10

typedef struct systemstate State;
struct systemstate {
    State *prev;

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

    State *next;

};

static STATE_ID state_beeing_rendered = NULL;

State *stack_head = NULL;
State *stack_tail = NULL;

void statesystem_init() // no longer needed?
{
	/*
    int i;
    for(i = 0; i<state_count; i++){
        states[i].prev = NULL;
        states[i].next = NULL;
        states[i].current_inner_state = 0;
        states[i].time_alive = 0;
        states[i].time_in_inner_state = 0;
    }
    */
}

STATE_ID statesystem_create_state(int inner_states,
        void (*on_enter)(),
        void (*pre_update)(),
        void (*post_update)(),
        void (*draw)(),
        void (*sdl_event)(),
        void (*on_leave)(),
        void (*destroy)())
{
	State *state = malloc(sizeof(*state));
    state->id = state;

    state->inner_states = inner_states;
	if (inner_states > 0) {
		//state->inner_update = malloc(sizeof(state->inner_update)*inner_states);
		//  state->inner_draw = malloc(sizeof(state->inner_draw)*inner_states);
	}
	state->on_enter = on_enter;
    state->pre_update = pre_update;
    state->post_update = post_update;
    state->sdl_event = sdl_event;
    state->draw = draw;
    state->on_leave = on_leave;
    state->destroy = destroy;

    state->prev = NULL;
    state->next = NULL;

    return state->id;
}
void statesystem_add_inner_state(STATE_ID state_id, int inner_state, void (*update)(), void (*draw)())
{
	State *state = (State *) state_id;

    state->inner_update[inner_state] = update;
    state->inner_draw[inner_state] = draw;
}

void statesystem_set_inner_state(STATE_ID state_id, int inner_state)
{
	State *state = (State *) state_id;

    state->time_in_inner_state = 0;
    state->current_inner_state = inner_state;
}

void statesystem_push_state(STATE_ID state_id)
{
	State *state = (State *) state_id;

    state->time_alive = 0;
    stack_head->next = state;
    stack_head->next->prev = stack_head;
    stack_head = stack_head->next;
}

void statesystem_pop_state()
{
    State *temp = stack_head;
    stack_head = stack_head->prev;
    temp->prev = NULL;
    stack_head->next = NULL;
}

void statesystem_set_state(STATE_ID state_id)
{
	State *stack_next, *state = stack_tail;

	while (state) {
		state->prev = NULL;
		stack_next = state->next;
		state->next = NULL;
		state = stack_next;
	}

	state = (State *) state_id;

    state->on_enter();
    state->time_alive = 0;
    stack_head = state;
    stack_tail = state;
}

void statesystem_update()
{
	if (stack_head->pre_update) {
		stack_head->pre_update();
	}

	if(stack_head->inner_states > 0 &&
			stack_head->inner_update[stack_head->current_inner_state]){
		stack_head->inner_update[stack_head->current_inner_state]();
	}

    if( stack_head->post_update){
        stack_head->post_update();
    }
}

void statesystem_draw()
{
    State *stack_temp = stack_tail;
    while(stack_temp){
    	state_beeing_rendered = stack_temp->id;
        stack_temp->draw();
        if(stack_temp->inner_states > 0 &&
                stack_temp->inner_draw[stack_temp->current_inner_state]){
            stack_temp->inner_draw[stack_temp->current_inner_state]();
        }
        stack_temp = stack_temp->next;

    }
    state_beeing_rendered = NULL;
}

void statesystem_destroy() // no longer needed?
{

}

void statesystem_free(STATE_ID state_id)
{
	State *state = (State *) state_id;

	if(state->destroy){
		state->destroy();
	}
	if(state->inner_states > 0){
		// free(state->inner_update);
		//  free(state->inner_draw);
	}
}

void statesystem_push_event(SDL_Event *event)
{
	if (stack_head->sdl_event) {
		stack_head->sdl_event(event);
	}
}

STATE_ID statesystem_get_render_state()
{
	return state_beeing_rendered;
}
