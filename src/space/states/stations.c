/*
 * stations.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include <stdio.h>
//#include "stations.h"
#include "levelscreen.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"
#include "../../engine/input/button.h"

STATE_ID state_stations;

char *txt_buttons[30];

static button *btn_stations;

static button home_button;

static level_ship *stations;
static int station_count = 2;


static float x_offset = 0;

static int tex_stars;


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

	float x1 = - (x_offset / 2) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT;
	float map[8] = {x1, 1,
					x2, 1,
					x1, 0,
					x2, 0};

	draw_texture(tex_stars,&cpvzero,map,GAME_WIDTH, GAME_HEIGHT,0);

	bmfont_render(FONT_BIG,0, 0, 0.7f*GAME_HEIGHT/2,"SPACE");
}


static void sdl_event(SDL_Event *event) {
	int i;
	switch (event->type) {
	case SDL_FINGERMOTION:
		if (!llist_contains(active_fingers,event->tfinger.fingerId)) {
			x_offset += event->tfinger.dx*GAME_WIDTH;
			for (i = 0; i < station_count; i++) {
				button_set_position(btn_stations[i], -(station_count - 1) / 2.0 * 600 + 600 * i + x_offset, 0);
			}
		}
		break;
	}

}


static void button_callback(void *data)
{
	levelscreen_change_to(data);
}

static void on_leave()
{
}

static void destroy()
{
}


void stations_init()
{
	statesystem_register(state_stations, 0);

	level_get_ships(&stations, &station_count);

	btn_stations = calloc(station_count, sizeof(button));

	Color col_back = {0.9,0.9,1,1};
	Color col_text = {1,1,1,1};

	int i;
	for (i = 0; i < station_count; i++) {
		char stri[10];
		sprintf(stri, "%d", i+1);
		btn_stations[i] = button_create(SPRITE_STATION_01, 0, stri, -(station_count - 1) / 2.0 * 600 + 600 * i, 0, 320, 320);
		button_set_data(btn_stations[i], &stations[i]);
		button_set_callback(btn_stations[i], button_callback);
		button_set_backcolor(btn_stations[i], col_back);
		button_set_frontcolor(btn_stations[i], col_text);
		button_set_animated(btn_stations[i], 1, 15);
		button_set_enlargement(btn_stations[i], 1.5);
		statesystem_register_touchable(state_stations, btn_stations[i]);
	}

	tex_stars = texture_load("stars.png");
}

