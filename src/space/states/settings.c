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

extern int debug_disable_render;
int player_camera_mode = 1;

#define SCROLL_WIDTH 1200

typedef enum {
	OPT_SOUND,
	OPT_MUSIC,
	OPT_ASSISTED_STEERING,
	OPT_DEBUG,
	OPT_TOGGLE_RENDER,
	OPT_DELETE,
	OPT_INVULNERABLE, // TODO: DISABLE DEBUG CHEAT!
	OPT_UNLOCK, // TODO: DISABLE DEBUG CHEAT!
	OPT_LOG, // TODO: DISABLE DEBUG CHEAT!
	OPT_TIME, // TODO: DISABLE DEBUG CHEAT!
	OPT_CAMERA, // camera mode

	OPTION_COUNT
} settings_option;

char str_options[OPTION_COUNT][50] = {
		"TOGGLE SOUND",
		"TOGGLE MUSIC",
		"ASSISTED STEERING",
        "DEBUG DRAW",
        "TOGGLE RENDER",
        "DELETE ALL",
		"CHEAT: INVULNERABLE",
		"CHEAT: UNLOCK ALL",
		"DEBUG: LOG",
		"DEBUG: Print time",
		"DEBUG: CAMERA"
};

static button btn_back;
static button btn_options[OPTION_COUNT];

static scroll_p scroller;

static cpVect boxsize;
static view *view_box;

int bit_settings;

static Color col_btn_checked = {51, 255, 51, 255};
static Color col_btn_unchecked = {255, 255, 255, 255};

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(STATE_ID state_prev)
{
}

static void pre_update(void)
{
	cpVect offset = scroll_get_offset(scroller);
	int i;
	for (i = 0; i < OPTION_COUNT; i++) {
		touch_place(btn_options[i], 0, GAME_HEIGHT/2*0.8+50 + offset.y - i * 150);
	}
}

static void post_update(void)
{
}

static void draw(void)
{
}

static int sdl_event(SDL_Event *event)
{
	return 0;
}

static void on_pause(void)
{
}

static void on_leave(STATE_ID state_next)
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

extern int debug_draw;
static void option_click(settings_option option)
{
#if !ARCADE_MODE
	SDL_Log("Option button #%d clicked :D (%s)", option, str_options[option]);
#endif

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
    case OPT_DEBUG:
        debug_draw = bit_toggle(option);
        button_set_backcolor(btn_options[option], debug_draw ? col_btn_checked : col_btn_unchecked);
        break;
    case OPT_TOGGLE_RENDER:
    	debug_disable_render = bit_toggle(option);
        button_set_backcolor(btn_options[option], debug_disable_render ? col_btn_checked : col_btn_unchecked);
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
	case OPT_TIME:
		debug_logtime = bit_toggle(option);
		button_set_backcolor(btn_options[option], bit_get_status(option) ? col_btn_checked : col_btn_unchecked);
		break;
	case OPT_CAMERA:
		player_camera_mode = player_camera_mode > 1 ? 0 : player_camera_mode + 1;
		SDL_ShowSimpleMessageBox(0, "Camera mode changed!", "You changed the camera mode!", NULL);
		break;
	case OPT_LOG:
		statesystem_push_state(state_log);
		break;
	default:
		//ERROR invalid case OPTion
		break;
	}
}

static void gui(view *cam)
{
	draw_color4f(0,0,0,0.5f);
	draw_box(1, cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);
	draw_color4f(0.2,0.4,0.8,0.8);
	draw_box(2, cpvzero,cpv(SCROLL_WIDTH,GAME_HEIGHT),0,1);

	draw_color(COL_WHITE);
	bmfont_center(FONT_SANS, cpv(0, GAME_HEIGHT/2 - 100), 1.5, "Settings");
}

void settings_init(void)
{
	statesystem_register(state_settings,0);

	view *view_main = state_view_get(state_settings, 0);
	view_main->GUI = gui;

	view_box = state_view_add(state_settings);
	boxsize = cpv(WINDOW_WIDTH * SCROLL_WIDTH / GAME_WIDTH , 0.8 * WINDOW_HEIGHT);
	//TODO fix transformation error when using non-centered viewport?
	view_set_port(view_box, cpvsub(cpv(WINDOW_WIDTH/2,WINDOW_HEIGHT/2),cpvmult(boxsize, 0.5)), boxsize, 0);  //-0.1*WINDOW_HEIGHT

	int i;
	for (i = 0; i < OPTION_COUNT; i++) {
		btn_options[i] = button_create(SPRITE_BTN2, 1, str_options[i], 0, 0, 800, 115);
        button_set_border(btn_options[i], 25);
		button_set_click_callback(btn_options[i], (void (*)(void *))option_click, NULL + i);
		button_set_hotkeys(btn_options[i], digit2scancode[(i+1) % 10], 0);
		state_register_touchable_view(view_box, btn_options[i]);
	}

	//TODO load options from file
	/* set predefined button states */
	option_click(OPT_SOUND); /* sound on */
	option_click(OPT_MUSIC); /* music on */
	option_click(OPT_MUSIC); /* music off */
	option_click(OPT_UNLOCK); /* everything unlocked */
	option_click(OPT_ASSISTED_STEERING); /* assisted steering activated by default */
	option_click(OPT_TIME);option_click(OPT_TIME);
#if !ARCADE_MODE
	//option_click(OPT_INVULNERABLE); /* assisted steering activated by default */
#endif

	/* disabled/unimplemented buttons */
	btn_options[OPT_UNLOCK]->enabled = 0;
	btn_options[OPT_DELETE]->enabled = 0;

	scroller = scroll_create(0,0,SCROLL_WIDTH,GAME_HEIGHT, 0.95, GAME_HEIGHT, 0, 0, 1);
	scroll_set_bounds(scroller, cpBBNew(0, 0, 0, 150 * (OPTION_COUNT-1)));
	state_register_touchable_view(view_box, scroller);

	btn_back = button_create(NULL, 0, "", 0,0,GAME_WIDTH,GAME_HEIGHT);
	button_set_hotkeys(btn_back, KEY_ESCAPE, KEY_RETURN_2);

	button_set_click_callback(btn_back, statesystem_pop_state, 0);
	state_register_touchable(this, btn_back);
}

