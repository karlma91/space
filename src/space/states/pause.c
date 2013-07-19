/*
 * pause.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

//#include "pause.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"
#include "../../engine/graphics/font.h"
#include "../../engine/input/button.h"

#include "space.h"


STATE_ID state_pause;

static button btn_space;
static button btn_retry;
static button btn_resume;

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
	draw_load_identity();

	draw_color4f(0,0,0,0.5f);
	draw_box(0,0,GAME_WIDTH,GAME_HEIGHT,0,1);

	draw_color4f(1,1,1,1);
	setTextSize(40);
	setTextAlign(TEXT_CENTER);
	font_drawText(0, 0, "PAUSED");
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


void pause_init()
{
	statesystem_register(state_pause, 0);

	btn_space = button_create(SPRITE_HOME, 0, "", -GAME_WIDTH/2 + 500, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_retry = button_create(SPRITE_RETRY, 0, "", GAME_WIDTH/2 - 500, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_resume = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);

	button_set_callback(btn_space, statesystem_set_state, state_stations);
	button_set_callback(btn_retry, space_restart_level, 0);
	button_set_callback(btn_resume, statesystem_set_state, state_space);

	btn_resume->visible = 0;

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_retry, 1.5);

	statesystem_register_touchable(this, btn_space);
	statesystem_register_touchable(this, btn_retry);
	statesystem_register_touchable(this, btn_settings);
	statesystem_register_touchable(this, btn_resume); /* full background button */
}

