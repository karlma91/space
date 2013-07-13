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

#define SCROLL_FRICTION 0.9f
static int scrolling = 0;
static float scroll_hs = 0;
static float scroll_vs = 0;
static float x_offset = 0;
static float y_offset = 0;

static int tex_title;
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

	if (!scrolling) {
		x_offset += scroll_hs;
		y_offset += scroll_vs;
	} else {
		scroll_hs *= SCROLL_FRICTION; // more friction
		scroll_vs *= SCROLL_FRICTION; // more friction
	}
	scroll_hs *= SCROLL_FRICTION;
	scroll_vs *= SCROLL_FRICTION;

	float x1 = - (x_offset / 2) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT;
	float y1 = (y_offset / 2) / GAME_HEIGHT;
	float y2 = y1 + 1;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};

	draw_texture(tex_stars,&cpvzero,map,GAME_WIDTH,GAME_HEIGHT,0);

	int i;
	for (i = 0; i < station_count; i++) {
		button_set_position(btn_stations[i], -(station_count - 1) / 2.0 * 650 + 1000 * i + x_offset, y_offset+(i-0.5)*270);
		button_render(btn_stations[i]);
	}

	cpVect pos = {0,0.7f*GAME_HEIGHT/2};
	draw_texture(tex_title, &pos, TEX_MAP_FULL, 1200, 300, 0);
}


static void sdl_event(SDL_Event *event) {
	switch (event->type) {
	case SDL_FINGERMOTION:
		if (!llist_contains(active_fingers,event->tfinger.fingerId)) {
			scrolling = 1;
			float dx = event->tfinger.dx*GAME_WIDTH;
			float dy = -event->tfinger.dy*GAME_HEIGHT;
			x_offset += dx;
			y_offset += dy;

			scroll_hs = scroll_hs*0.5 + 0.5*dx;
			scroll_vs = scroll_vs*0.5 + 0.5*dy;
		}
		break;
	case SDL_FINGERUP:
			scrolling = 0;
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
		float size = i ? -10 : 30;
		btn_stations[i] = button_create(SPRITE_STATION_01, 0, "", -(station_count - 1) / 2.0 * 650 + 650 * i, -i*70, 320 + size, 320 + size);
		button_set_data(btn_stations[i], &stations[i]);
		button_set_callback(btn_stations[i], button_callback);
		button_set_backcolor(btn_stations[i], col_back);
		button_set_frontcolor(btn_stations[i], col_text);
		button_set_animated(btn_stations[i], 1, (i ? 18 : 15));
		button_set_enlargement(btn_stations[i], 1.5);
		button_set_visibility(btn_stations[i],0);
		statesystem_register_touchable(state_stations, btn_stations[i]);
	}

	tex_title = texture_load("space_title.png");
	tex_stars = texture_load("stars.png");
}

