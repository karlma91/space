/*
 * mainmenu.c
 *
 *  Created on: 3. juli 2013
 *      Author: Mathias
 */

#include "main.h"
#include "game.h"
#include "state.h"
#include "draw.h"
#include "font.h"
#include "space.h"
#include "gameover.h"

STATE_ID state_mainmenu;

static enum {
	PLAY_DEMO, MULTIPLAYER, SETTINGS, HIGHSCORES, //TMP
	BUTTON_COUNT
} mainmenu;

char txt_buttons[BUTTON_COUNT][30] = {"PLAY DEMO", "2-PLAYER", "SETTINGS", "HIGHSCORES"};

void (*call_buttons[BUTTON_COUNT])(void) = {space_start_demo, space_start_multiplayer, NULL, gameover_showhighscores};

button buttons[BUTTON_COUNT];

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter() {
}

static void pre_update() {
}

static void post_update() {
}

static void draw() {
	static float timer;
	timer +=dt;

	setTextSize(120);
	setTextAlign(TEXT_CENTER);
	drawStars();
	draw_color(draw_col_rainbow((int)(timer*1000)));
	font_drawText(0,0.7f*GAME_HEIGHT/2, "SPACE");

	int i;

	draw_color4f(1,1,1,1);
	bmfont_render(FONT_NORMAL,TEXT_CENTER, 0, 0.6f*GAME_HEIGHT/2,"Grumpy wizards make toxic brew for the evil Queen and Jack\n%d",123456789);
	bmfont_render(FONT_NORMAL,TEXT_CENTER, 0, 0.5f*GAME_HEIGHT/2,"!\"#$%&\'()*+,-./:;<=>?@[\\]^_{|}~");
	bmfont_render(FONT_BIG,TEXT_CENTER, 0, 0.3f*GAME_HEIGHT/2,"Grumpy wizards make toxic brew for the evil Queen and Jack\n%d",123456789);
	bmfont_render(FONT_BIG,TEXT_CENTER, 0, 0.4f*GAME_HEIGHT/2,"!\"#$%&\'()*+,-./:;<=>?@[\\]^_{|}~");

	for (i = 0; i < BUTTON_COUNT; i++) {
		draw_color4f(0,0.2,0.9,1);
		button_render(buttons[i]);
	}
}

static void sdl_event(SDL_Event *event) {
	int i;
	switch (event->type) {
	case SDL_FINGERDOWN:
		for (i = 0; i < BUTTON_COUNT; i++) {
			if (button_finger_down(buttons[i], &event->tfinger)) {
				break;
			}
		}
		break;
	case SDL_FINGERMOTION:
		for (i = 0; i < BUTTON_COUNT; i++) {
			button_finger_move(buttons[i], &event->tfinger);
		}
		break;
	case SDL_FINGERUP:
		for (i = 0; i < BUTTON_COUNT; i++) {
			if (button_finger_up(buttons[i], &event->tfinger)) {
				//TODO add callback function for buttons
				//button_call(buttons[i]);
				break;
			}
		}
		break;
	}
}

static void on_leave() {
}

static void destroy() {
}

static void onclicktmp() {
	statesystem_set_state(state_menu);
}

void mainmenu_init() {
	statesystem_register(state_mainmenu, 0);

	int i;
	for (i = 0; i < BUTTON_COUNT; i++) {
		buttons[i] = button_create(SPRITE_BUTTON, 1, txt_buttons[i], 0, -i*160, 500, 140);
		button_set_callback(buttons[i], call_buttons[i]);
	}
}

