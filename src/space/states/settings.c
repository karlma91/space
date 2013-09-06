/*
 * settings.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

//#include "settings.h"

#include "../game.h"
#include "we_defstate.h"

STATE_ID state_settings;

#define SCROLL_WIDTH 1200

typedef enum {
	OPT_SOUND,
	OPT_MUSIC,
	OPT_ASSISTED_STEERING,
	OPT_DELETE,
	OPT_INVULNERABLE, // TODO: DISABLE DEBUG CHEAT!
	OPT_UNLOCK, // TODO: DISABLE DEBUG CHEAT!

	OPTION_COUNT
} settings_option;

char str_options[OPTION_COUNT][50] = {
		"TOGGLE SOUND",
		"TOGGLE MUSIC",
		"ASSISTED STEERING",
		"DELETE ALL",
		"CHEAT: INVULNERABLE",
		"CHEAT: UNLOCK ALL",
};

static button btn_back;
static button btn_options[OPTION_COUNT];

static scroll_p scroller;


int bit_settings;

Color col_btn_checked = {0.2, 1, 0.2, 1};
Color col_btn_unchecked = {1, 1, 1, 1};

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
}

static void pre_update(void)
{
}

static void post_update(void)
{
}

static void draw(void)
{
	float yoffset = scroll_get_yoffset(scroller);

	draw_load_identity();

	draw_color4f(0,0,0,0.5f);
	draw_box(0,0,GAME_WIDTH,GAME_HEIGHT,0,1);
	draw_color4f(0.1,0.1,0.3,1);
	draw_box(0,0,SCROLL_WIDTH,GAME_HEIGHT,0,1);


	draw_color4f(1,1,1,1);

	setTextSize(70);
	setTextAlign(TEXT_CENTER);
	font_drawText(0, GAME_HEIGHT/2 - 80 + yoffset, "SETTINGS");

	int i;
	for (i = 0; i < OPTION_COUNT; i++) {
		touch_place(btn_options[i], 0, -i * 160 + GAME_HEIGHT/5 + yoffset);
	}
}

static void sdl_event(SDL_Event *event)
{
}

static void on_pause(void)
{
}

static void on_leave(void)
{
}

static void destroy(void)
{
}

static int bit_toggle(int bit_index)
{
	bit_settings ^= 1 << bit_index;
	return (bit_settings >> bit_index) & 0x1;
}

static int bit_get_status(int bit_index)
{
	return (bit_settings >> bit_index) & 0x1;
}

static void option_click(settings_option option)
{
	SDL_Log("Option button #%d clicked :D (%s)", option, str_options[option]);


	switch (option)
	{
	case OPT_SOUND:
		if (bit_toggle(option)) {
			sound_unmute();
			button_set_backcolor(btn_options[option], col_btn_checked);
		} else {
			sound_mute();
			button_set_backcolor(btn_options[option], col_btn_unchecked);
		}
		break;
	case OPT_MUSIC:
		if (bit_toggle(option)) {
			sound_music_unmute();
			button_set_backcolor(btn_options[option], col_btn_checked);
		} else {
			sound_music_mute();
			button_set_backcolor(btn_options[option], col_btn_unchecked);
		}
		break;
	case OPT_ASSISTED_STEERING:
		player_assisted_steering = bit_toggle(option);
		button_set_backcolor(btn_options[option], player_assisted_steering ? col_btn_checked : col_btn_unchecked);
		break;
	case OPT_DELETE:
		/* TODO implement this unimplemented option! */
		break;

		// TODO: DISABLE DEBUG CHEAT!
	case OPT_INVULNERABLE:
		player_cheat_invulnerable = bit_toggle(option);
		button_set_backcolor(btn_options[option], player_cheat_invulnerable ? col_btn_checked : col_btn_unchecked);
		break;
	case OPT_UNLOCK:
		/* TODO implement this unimplemented option! */
		bit_toggle(option);
		button_set_backcolor(btn_options[option], bit_get_status(option) ? col_btn_checked : col_btn_unchecked);
		break;
	default:
		//ERROR invalid case OPTion
		break;
	}
}

void settings_init(void)
{
	statesystem_register(state_settings,0);

	int i;
	for (i = 0; i < OPTION_COUNT; i++) {
		btn_options[i] = button_create(SPRITE_BUTTON, 1, str_options[i], -1,-1, 800, 115);
		button_set_callback(btn_options[i], (void (*)(void *))option_click, NULL + i);
		button_set_hotkeys(btn_options[i], digit2scancode[(i+1) % 10], 0);
		statesystem_register_touchable(this, btn_options[i]);
	}

	//TODO load options from file
	/* set predefined button states */
	option_click(OPT_SOUND); /* sound on */
	option_click(OPT_MUSIC); /* music on */
	option_click(OPT_UNLOCK); /* everything unlocked */
	option_click(OPT_ASSISTED_STEERING); /* assisted steering activated by default */

	/* disabled/unimplemented buttons */
	btn_options[OPT_UNLOCK]->enabled = 0;
	btn_options[OPT_DELETE]->enabled = 0;

	scroller = scroll_create(0,0,SCROLL_WIDTH,GAME_HEIGHT, 0.95, GAME_HEIGHT);
	statesystem_register_touchable(state_settings, scroller);

	btn_back = button_create(NULL, 0, "", 0,0,GAME_WIDTH,GAME_HEIGHT);
	button_set_hotkeys(btn_back, KEY_ESCAPE, KEY_RETURN_2);

	button_set_callback(btn_back, statesystem_pop_state, 0);
	statesystem_register_touchable(this, btn_back);
	btn_back->visible = 0;
}
