/*
 * stations.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include "stdio.h"
#include "stations.h"
#include "levelscreen.h"

#include "main.h"
#include "game.h"
#include "button.h"
#include "level.h"
#include "state.h"

STATE_ID state_stations;

char *txt_buttons[30];

static button *buttons;

static button home_button;

static level_ship *stations;
static int station_count = 2;

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
	for (i = 0; i < station_count; i++) {
		draw_color4f(0,0.2,0.9,1);
		button_render(buttons[i]);
	}
}


static void sdl_event(SDL_Event *event) {
	int i;
	switch (event->type) {
	case SDL_FINGERDOWN:
		for (i = 0; i < station_count; i++) {
			if (button_finger_down(buttons[i], &event->tfinger)) {
				break;
			}
		}
		break;
	case SDL_FINGERMOTION:
		for (i = 0; i < station_count; i++) {
			button_finger_move(buttons[i], &event->tfinger);
		}
		break;
	case SDL_FINGERUP:
		for (i = 0; i < station_count; i++) {
			if (button_finger_up(buttons[i], &event->tfinger)) {
				//TODO add callback function for buttons
				//button_call(buttons[i]);
				break;
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

	buttons = calloc(station_count, sizeof(button));

	int i;
	for (i = 0; i < station_count; i++) {
		char stri[10];
		sprintf(stri, "TEKST %d", i+1);
		buttons[i] = button_create(SPRITE_BUTTON, 1, stri, 0, -i*160, 140, 140);
		button_set_data(buttons[i], &stations[i]);
		button_set_callback(buttons[i], button_callback);
	}

}

