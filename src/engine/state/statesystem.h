/*
 * statesystem.h
 *
 *  Created on: Mar 22, 2013
 *      Author: karlmka
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_

#include "SDL_events.h"

typedef void * STATE_ID;

typedef struct {
    void (*on_enter)(void);
    void (*pre_update)(void);
    void (*post_update)(void);
    void (*draw)(void);
    void (*sdl_event)(SDL_Event *event);
    void (*on_pause)(void);
    void (*on_leave)(void);
    void (*destroy)(void);
} state_funcs;


/**
 * standard functions
 */
void statesystem_init(void);
void statesystem_update(void);
void statesystem_draw(void);
void statesystem_pause(void);
void statesystem_destroy(void);

/**
 * statesystem functions
 */

STATE_ID statesystem_get_render_state(void);

STATE_ID statesystem_create_state(int inner_states, state_funcs *funcs);

void statesystem_add_inner_state(STATE_ID state, int inner_state, void (*update)(void), void (*draw)(void));
void statesystem_set_inner_state(STATE_ID state, int inner_state);

void statesystem_push_state(STATE_ID state);
void statesystem_pop_state(void);
void statesystem_set_state(STATE_ID state);

void statesystem_call_update(STATE_ID state_id);
void statesystem_push_event(SDL_Event *event);

void statesystem_register_touchable(STATE_ID state, void *touchable);

void statesystem_free(STATE_ID state);

#endif /* STATESYSTEM_H_ */
