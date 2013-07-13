/*
 * leveldone.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

//#include "leveldone.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"

#include "../../engine/graphics/draw.h"
#include "../../engine/graphics/font.h"

#include "../../engine/input/button.h"

#include "space.h"

STATE_ID state_leveldone;

sprite spr_star;

static button btn_space;
static button btn_retry;
static button btn_next;

static cpVect star_1 = {-350,100};
static cpVect star_2 = {0,100};
static cpVect star_3 = {350,100};

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
	draw_color4f(1,1,1,1);
	draw_box(0,0,1220,920,0,1);
	draw_color4f(0,0,0,1);
	draw_box(0,0,1200,900,0,1);

	draw_color4f(1,1,1,1);
	setTextAlign(TEXT_CENTER);
	setTextSize(40);
	font_drawText(0,300,"LEVEL CLEARED");

	button_render(btn_space);
	button_render(btn_retry);
	button_render(btn_next);


	sprite_render(&spr_star, &star_1, 0);
	sprite_render(&spr_star, &star_2, 0);
	sprite_render(&spr_star, &star_3, 0);
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

	btn_space = button_create(SPRITE_BUTTON, 1, "SPACE", -380, -320, 200, 150);
	btn_retry = button_create(SPRITE_BUTTON, 1, "RETRY", 0, -320, 200, 150);
	btn_next = button_create(SPRITE_BUTTON, 1, "NEXT", 380, -320, 200, 150);

	button_set_data(btn_space, state_stations);

	button_set_callback(btn_space, statesystem_set_state);
	button_set_callback(btn_retry, space_restart_level);
	button_set_callback(btn_next, space_next_level);

	statesystem_register_touchable(state_leveldone, btn_space);
	statesystem_register_touchable(state_leveldone, btn_retry);
	statesystem_register_touchable(state_leveldone, btn_next);

	sprite_create(&spr_star,SPRITE_STAR,250,250,0);
}

