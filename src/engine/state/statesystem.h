/*
 * statesystem.h
 *
 *  Created on: Mar 22, 2013
 *      Author: karlmka
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_

#include "SDL_events.h"
#include "we_graphics.h"
#include "chipmunk.h"
#include "../components/object.h"
#include "../graphics/particles.h"

extern cpSpace *current_space;
extern object_system *current_objects;
extern particle_system *current_particles;
extern view * current_view;

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

void state_enable_objects(STATE_ID state_id, int enabled);
void state_enable_particles(STATE_ID state_id, int enabled);

view *state_view_add(STATE_ID state_id);
view *state_view_get(STATE_ID state_id, int index);
view *state_view_enable(STATE_ID state_id, int index, int enabled);

void state_add_inner_state(STATE_ID state, int inner_state, void (*update)(void), void (*draw)(void));
void state_set_inner_state(STATE_ID state, int inner_state);

void statesystem_push_state(STATE_ID state);
void statesystem_pop_state(void);
void statesystem_set_state(STATE_ID state);

void statesystem_call_update(STATE_ID state_id);
void statesystem_push_event(SDL_Event *event);

void state_register_touchable(STATE_ID state_id, void *touchable);
void state_register_touchable_view(view *cam, void *touchable);

#endif /* STATESYSTEM_H_ */

