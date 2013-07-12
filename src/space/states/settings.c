/*
 * settings.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

//#include "settings.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"

STATE_ID state_settings;


/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter()
{
}

static void pre_update()
{
}

static void post_update()
{
}

static void draw()
{
}

static void sdl_event(SDL_Event *event)
{
}

static void on_leave()
{
}

static void destroy()
{
}


void settings_init()
{
	statesystem_register(state_settings,0);
}

