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
#include "../../engine/input/scroll.h"

STATE_ID state_stations;

char *txt_buttons[30];

static button *btn_stations;
static button btn_home;

static level_ship *stations;
static int station_count = 2;

static scroll_p scroller;

static int tex_stars;
static int tex_stars_2;

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

static void sdl_event(SDL_Event *event)
{
}

static void draw()
{
	draw_color4f(1,1,1,1);

	float xoffset = scroll_get_xoffset(scroller);
	float yoffset = scroll_get_yoffset(scroller);

	{
	float x1 = - (xoffset / 7) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT;
	float y1 = (yoffset / 7) / GAME_HEIGHT;
	float y2 = y1 + 1;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars,&cpvzero,map,GAME_WIDTH,GAME_HEIGHT,0);
	}
	{
	float x1 = - (xoffset / 4) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT * 2;
	float y1 = (yoffset / 4) / GAME_HEIGHT;
	float y2 = y1 + 1.0 * 2;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars_2,&cpvzero,map,GAME_WIDTH,GAME_HEIGHT,0);
	}
	{
	float x1 = - (xoffset / 3.6) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT *1;
	float y1 = (yoffset / 3.6) / GAME_HEIGHT;
	float y2 = y1 + 1.0 * 1;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars_2,&cpvzero,map,GAME_WIDTH,GAME_HEIGHT,0);
	}
	{
	float x1 = - (xoffset / 3.2) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT * 0.5;
	float y1 = (yoffset / 3.2) / GAME_HEIGHT;
	float y2 = y1 + 1 * 0.5;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars_2,&cpvzero,map,GAME_WIDTH,GAME_HEIGHT,0);
	}

	int i;
	for (i = 0; i < station_count; i++) {
		touch_place(btn_stations[i], -(station_count - 1) / 2.0 * 650 + 1000 * i + xoffset, yoffset+(i-0.5)*270);
		btn_stations[i]->calls->render(btn_stations[i]);
		//TODO tegne minste antall stjerner av levlene i romstasjonen under den?
		//TODO skrive antall levler klart?
	}

	//cpVect pos = {0,0.7f*GAME_HEIGHT/2};
	//draw_texture(tex_title, &pos, TEX_MAP_FULL, 1200, 300, 0);
}

static void button_callback(void *data)
{
	levelscreen_change_to(data);
}

static void on_pause()
{
}

static void on_leave()
{
}

static void destroy()
{
}

static void open_upgrades()
{
	statesystem_push_state(state_store);
}

void stations_init()
{
	statesystem_register(state_stations, 0);

	level_get_ships(&stations, &station_count);

	btn_stations = calloc(station_count, sizeof(button));

	Color col_back = {1,1,1,1};
	//Color col_text = {1,1,1,1};

	btn_home = button_create(SPRITE_PLAYER, 0, "", 0, 0, 250, 250);
	button_set_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 2);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	statesystem_register_touchable(this, btn_home);

	int i;
	for (i = 0; i < station_count; i++) {
		char stri[10];
		sprintf(stri, "%d", i+1);
		float size = 350 + (i ? -50 : 50);
		btn_stations[i] = button_create(SPRITE_STATION_01, 0, "", -1,-1, size, size);
		button_set_callback(btn_stations[i], button_callback, &stations[i]);
		button_set_backcolor(btn_stations[i], col_back);
		button_set_animated(btn_stations[i], 1, (i ? 18 : 15));
		button_set_enlargement(btn_stations[i], 1.5);
		button_set_hotkeys(btn_stations[i], digit2scancode[(i+1) % 10], 0);
		btn_stations[i]->visible = 0;

		statesystem_register_touchable(this, btn_stations[i]);
	}

	statesystem_register_touchable(this, btn_settings);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000); // max 4 000 gu / sec
	statesystem_register_touchable(this, scroller);

	tex_stars = texture_load("stars.jpg");
	tex_stars_2 = texture_load("stars_2.png");
}

