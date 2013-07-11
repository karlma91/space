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


static button levels[10];

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
	int i;
	for (i = 0; i < level_count; i++) {
		draw_color4f(0,0.2,0.9,1);
		button_render(levels[i]);
	}
}

static void sdl_event(SDL_Event *event) {
	int i;
	switch (event->type) {
	case SDL_FINGERDOWN:
		for (i = 0; i < level_count; i++) {
			if (button_finger_down(levels[i], &event->tfinger)) {
				break;
			}
		}
		break;
	case SDL_FINGERMOTION:
		for (i = 0; i < level_count; i++) {
			button_finger_move(levels[i], &event->tfinger);
		}
		break;
	case SDL_FINGERUP:
		for (i = 0; i < level_count; i++) {
			if (button_finger_up(levels[i], &event->tfinger)) {
				//TODO add callback function for buttons
				//button_call(buttons[i]);
				break;
			}
		}
		break;
	}
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
	int i;
	for (i = 0; i < 10; i++) {
		char stri[10];
		sprintf(stri, "%d", i+1);
		levels[i] = button_create(SPRITE_BUTTON, 1, stri, -GAME_WIDTH/3 + i*160, 200, 60, 140);
		button_set_data(levels[i], i+1);
		button_set_callback(levels[i], button_callback);
	}
}

