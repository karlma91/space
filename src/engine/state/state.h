/*
 * state.h
 *
 * Denne filen skal inkluderes av all c-filer som beskriver en enkelt state
 * (statesystem.h er allerede inkludert i denne filen), andre filer som
 * behandler states skal kun inkludere statesystem.h
 *
 *  Created on: 3. juli 2013
 *      Author: Mathias
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_
#include "SDL.h"
#include "statesystem.h"
#endif

/* static prototypes shared by all states */
static void on_enter(void);
static void pre_update(void);
static void post_update(void);
static void draw(void);
static void sdl_event(SDL_Event *event);
static void on_pause(void);
static void on_leave(void);
static void destroy(void);

static state_funcs state_functions = {
		on_enter,
		pre_update,
		post_update,
		draw,
		sdl_event,
		on_pause,
		on_leave,
		destroy,
};

static STATE_ID this;

/* Macro for registration of state */
#define statesystem_register(STATE_NAME, INNER_STATES) STATE_NAME = statesystem_create_state(INNER_STATES, &state_functions); this = STATE_NAME;
