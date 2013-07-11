/*
 * leveldone.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

#include "leveldone.h"

#include "main.h"
#include "game.h"
#include "state.h"

STATE_ID state_leveldone;


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


void leveldone_init()
{
	statesystem_register(state_leveldone,0);
}

