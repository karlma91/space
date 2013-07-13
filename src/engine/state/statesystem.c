#include "stdlib.h"
#include "stdio.h"
#include "statesystem.h"
#include "../data/llist.h"
#include "../input/touch.h"

#define MAX_INNER_STATES 10

LList states;

typedef struct systemstate State;
struct systemstate {
    State *prev;
    State *next;

    STATE_ID id;

    int inner_states;
    int current_inner_state;
    float time_alive;
    float time_in_inner_state;

    LList touch_objects;

    void (*inner_update[MAX_INNER_STATES])();
    void (*inner_draw[MAX_INNER_STATES])();

    state_funcs call;
};

static STATE_ID state_beeing_rendered = NULL;

State *stack_head = NULL;
State *stack_tail = NULL;

void statesystem_init() // no longer needed?
{
	states = llist_create();
}

STATE_ID statesystem_create_state(int inner_states, state_funcs *funcs)
{
	State *state = malloc(sizeof(*state));
    state->id = state;

    state->inner_states = inner_states;
	if (inner_states > 0) {
		//state->inner_update = malloc(sizeof(state->inner_update)*inner_states);
		//  state->inner_draw = malloc(sizeof(state->inner_draw)*inner_states);
	} else if (inner_states > MAX_INNER_STATES) {
		fprintf(stderr, "Number of inner states(%d) exceeded MAX_INNER_STATES (%d)\n", inner_states, MAX_INNER_STATES);
		exit(-1);
	}
	state->call.on_enter = funcs->on_enter;
    state->call.pre_update = funcs->pre_update;
    state->call.post_update = funcs->post_update;
    state->call.draw = funcs->draw;
    state->call.sdl_event = funcs->sdl_event;
    state->call.on_leave = funcs->on_leave;
    state->call.destroy = funcs->destroy;

    state->touch_objects = llist_create();

    state->prev = NULL;
    state->next = NULL;

    llist_add(states, (void*)state);

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

    stack_head->call.on_enter();
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

    state->call.on_enter();
    state->time_alive = 0;
    stack_head = state;
    stack_tail = state;
}

void statesystem_update()
{
	if (stack_head->call.pre_update) {
		stack_head->call.pre_update();
	}

	LList list = stack_head->touch_objects;
	llist_begin_loop(list);
	while(llist_hasnext(list)) {
		void *touchy = llist_next(list);
		((touch_calls *) (*(void **)touchy))->update(touchy);
	}
	llist_end_loop(list);

	if(stack_head->inner_states > 0 &&
			stack_head->inner_update[stack_head->current_inner_state]){
		stack_head->inner_update[stack_head->current_inner_state]();
	}

    if( stack_head->call.post_update){
        stack_head->call.post_update();
    }
}

void statesystem_draw()
{
    State *state = stack_tail;
    while(state){
    	state_beeing_rendered = state->id;
    	state->call.draw();
    	if(state->inner_states > 0 &&
    			state->inner_draw[state->current_inner_state]){
    		state->inner_draw[state->current_inner_state]();
    	}

    	LList list = state->touch_objects;
    	llist_begin_loop(list);
    	while(llist_hasnext(list)) {
    		void *touchy = llist_next(list);
    		if (button_is_visible(touchy)) {
    			((touch_calls *) (*(void **)touchy))->render(touchy);
    		}
    	}
    	llist_end_loop(list);

    	state = state->next;

    }

    state_beeing_rendered = NULL;
}

void statesystem_destroy()
{
	llist_begin_loop(states);
	while (llist_hasnext(states)) {
		statesystem_free(llist_next(states));
	}
	llist_end_loop(states);

	llist_destroy(states);
}

void statesystem_free(STATE_ID state_id)
{
	State *state = (State *) state_id;

	if(state->call.destroy){
		state->call.destroy();
	}
	if(state->inner_states > 0){
		// free(state->inner_update);
		//  free(state->inner_draw);
	}

	llist_destroy(state->touch_objects);
}

void statesystem_push_event(SDL_Event *event)
{
	if (stack_head->call.sdl_event) {
		stack_head->call.sdl_event(event);
	}

	LList list = stack_head->touch_objects;
	llist_begin_loop(list);
	switch(event->type) {
	case SDL_FINGERDOWN:
		while(llist_hasnext(list)) {
			void *touchy = llist_next(list);
			if (button_is_enabled(touchy)) {
				if (((touch_calls *) (*(void **)touchy))->touch_down(touchy, &event->tfinger))
					break;
			}
		}
		break;
	case SDL_FINGERMOTION:
		while(llist_hasnext(list)) {
			void *touchy = llist_next(list);
			if (button_is_enabled(touchy)) {
				if (((touch_calls *) (*(void **)touchy))->touch_motion(touchy, &event->tfinger))
					break;
			}
		}
		break;
	case SDL_FINGERUP:
		while(llist_hasnext(list)) {
			void *touchy = llist_next(list);
			if (button_is_enabled(touchy)) {
				if (((touch_calls *) (*(void **)touchy))->touch_up(touchy, &event->tfinger))
					break;
			}
		}
		break;
	}
	llist_end_loop(list);
}

STATE_ID statesystem_get_render_state()
{
	return state_beeing_rendered;
}


void statesystem_register_touchable(STATE_ID state_id, void *touchable)
{
	State *state = (State *) state_id;

	llist_add(state->touch_objects, touchable);
}
