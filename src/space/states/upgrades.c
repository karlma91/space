/*
 * upgrades.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

//#include "upgrades.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"
#include "../../engine/graphics/font.h"
#include "../../engine/graphics/draw.h"

STATE_ID state_upgrades;

static rect box = {0,0,-1,GAME_HEIGHT};
static int tex_sketch;
static button btn_space;

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
	draw_color4f(0.1,0.2,0.4,1);
	draw_box(box.x,box.y,box.w,box.h,0,1);

	draw_color4f(1,1,1,1);
	draw_texture(tex_sketch, &cpvzero,TEX_MAP_FULL, box.w,box.h,0);

	draw_color4f(1,1,1,1);
	setTextAlign(TEXT_CENTER);
	setTextSize(50);
	font_drawText(0, box.y+box.h / 2 - 60, "STORE");
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


void upgrades_init()
{
	statesystem_register(state_upgrades,0);

	box.w = GAME_WIDTH;

	btn_space = button_create(NULL, 0, "",   0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_callback(btn_space, statesystem_set_state, state_stations);
	statesystem_register_touchable(this, btn_space);

	tex_sketch = texture_load("tmp_upgrades.png");
}

