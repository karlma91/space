/*
 * levelscreen.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include "levelscreen.h"

#include "main.h"
#include "game.h"
#include "state.h"

STATE_ID state_levelscreen;


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


void levelscreen_init()
{
	statesystem_register(state_levelscreen,0);
}

