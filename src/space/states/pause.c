/*
 * pause.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */

#include "../game.h"
#include "../obj/object_types.h"
#include "../upgrades.h"
#include "we_defstate.h"

#include "space.h"

static char *input_buffer;

STATE_ID state_pause;

static button btn_space;
static button btn_retry;
static button btn_resume;
static button btn_input;

static int valid_cheat = 0;

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
	draw_load_identity();

	draw_color_rgbmulta4b(0,0,0,156);
	draw_box(4, cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);

	if ((int)engine_time & 0x3) {
		draw_color(COL_WHITE);
		bmfont_center(FONT_SANS, cpv(0, 0), 1, "Tap to resume");
	}

	int keyboard_shown = SDL_IsScreenKeyboardShown(window);
	draw_color4f(!valid_cheat,keyboard_shown,valid_cheat ^ keyboard_shown,1);
	setTextAlign(TEXT_LEFT);
	bmfont_left(FONT_SANS, cpv(btn_input->get.x+30, btn_input->get.y-150), 0.5, input_buffer);
}

static void cheat(void)
{
	/* CHEAT */
	object_id *obj = object_by_name(input_buffer);
	obj_player *player = (obj_player *)instance_first(obj_id_player);
	if (!player) return;
	valid_cheat = (obj != NULL);
	player->bullet_type = obj ? obj : object_by_name(weapons[weapon_index].obj_name); /* override player's bullet object */
}

static int sdl_event(SDL_Event *event)
{
	switch(event->type) {
	case SDL_TEXTINPUT:
        /* Add new text onto the end of our text */
        strcat(input_buffer, event->text.text);
        cheat();
		break;
	case SDL_TEXTEDITING:
        /*
        Update the composition text.
        Update the cursor position.
        Update the selection length (if any).
        */
        fprintf(stderr, "TEXTEDIT->entered: %s\n", event->text.text);
        /*
        composition = event.edit.text;
        cursor = event.edit.start;
        selection_len = event.edit.length;
        */
		break;
	}
	return 0;
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


static void resume(void *unused)
{
	SDL_StopTextInput();
	statesystem_set_state(state_space);
}

static void input(void *unused)
{
	memset(input_buffer,0,1000);
	SDL_StartTextInput();
	SDL_Rect r = {btn_input->get.x,btn_input->get.y-200, 400, 400};
	SDL_SetTextInputRect(&r);
}

void pause_init(void)
{
	statesystem_register(state_pause, 0);

	input_buffer = calloc(sizeof (char), 1000);

	btn_space = button_create(SPRITE_BTN_HOME, 0, "", -GAME_WIDTH/2 + 500, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_retry = button_create(SPRITE_BTN_RETRY, 0, "", GAME_WIDTH/2 - 500, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_input = button_create(NULL, 0, "", -GAME_WIDTH/2+50, GAME_HEIGHT/2-50, 100, 100);
	btn_resume = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);

	button_set_click_callback(btn_space, statesystem_set_state, state_stations);
	button_set_click_callback(btn_retry, space_restart_level, 0);
	button_set_click_callback(btn_input, input, 0);
	button_set_click_callback(btn_resume, resume, 0);
	button_set_hotkeys(btn_resume, KEY_ESCAPE, KEY_RETURN_2);
	button_set_hotkeys(btn_retry, SDL_SCANCODE_SPACE, 0);
	button_set_hotkeys(btn_space, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_HOME);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_retry, 1.5);

	state_register_touchable(this, btn_space);
	state_register_touchable(this, btn_retry);
	state_register_touchable(this, btn_settings);
	state_register_touchable(this, btn_input);
	state_register_touchable(this, btn_resume); /* full background button */
}

