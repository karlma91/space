/*
 * statesystem.h
 *
 *  Created on: Mar 22, 2013
 *      Author: karlmka
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_

#include "SDL.h"

typedef int STATE_ID;

/**
 * standard functions
 */
void statesystem_init();
void statesystem_update();
void statesystem_draw();
void statesystem_destroy();

/**
 * statesystem functions
 */

STATE_ID statesystem_get_render_state();

STATE_ID statesystem_add_state(int inner_states,
        void (*on_enter)(),
        void (*pre_update)(),
        void (*post_update)(),
        void (*draw)(),
        void (*sdl_event)(),
        void (*on_leave)(),
        void (*destroy)());

void statesystem_add_inner_state(STATE_ID state, int inner_state, void (*update)(), void (*draw)());

void statesystem_set_inner_state(STATE_ID state, int inner_state);

void statesystem_push_state(STATE_ID state);
void statesystem_pop_state();
void statesystem_set_state(STATE_ID state);

void statesystem_push_event(SDL_Event *event);

#endif /* STATESYSTEM_H_ */
