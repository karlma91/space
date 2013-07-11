/*
 * levelscreen.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include "stdio.h"
#include "space.h"
#include "levelscreen.h"

#include "main.h"
#include "game.h"
#include "state.h"

STATE_ID state_levelscreen;

#define MAX_LEVELS 10
static button btn_levels[MAX_LEVELS];

static int level_count = 0;
static level_ship *current_ship;


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
	bmfont_render(FONT_BIG,0, 0, 0.7f*GAME_HEIGHT/2,"SPACE");
}

static void sdl_event(SDL_Event *event) {
}

static void on_leave()
{
}

static void destroy()
{
}

void levelscreen_change_to(level_ship * ship)
{
	current_ship = ship;
	level_count = ship->count;

	int i;
	for (i = 0; i < level_count; i++) {
		button_set_visibility(btn_levels[i], 1);
	}
	for (;i < MAX_LEVELS; i++) {
		button_set_visibility(btn_levels[i], 0);
	}

	statesystem_push_state(state_levelscreen);
}

static void button_callback(void *data)
{
	space_start_demo();
	space_init_level(current_ship->id, (int)data);
}

void levelscreen_init()
{
	statesystem_register(state_levelscreen,0);

	Color col_back = {0,0.2,0.9,1};
	Color col_text = {1,1,1,1};

	int i;
	for (i = 0; i < MAX_LEVELS; i++) {
		char stri[MAX_LEVELS];
		sprintf(stri, "%d", i+1);
		btn_levels[i] = button_create(SPRITE_BUTTON, 1, stri, -GAME_WIDTH/3 + i*160, 200, 60, 140);

		button_set_data(btn_levels[i], i+1);
		button_set_callback(btn_levels[i], button_callback);
		button_set_backcolor(btn_levels[i], col_back);
		button_set_frontcolor(btn_levels[i], col_text);
		statesystem_register_touchable(state_levelscreen, btn_levels[i]);
	}
}

