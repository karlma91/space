/*
 * leveldone.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */
#include "../game.h"
#include "leveldone.h"
#include "we_defstate.h"
#include "space.h"

STATE_ID state_leveldone;

sprite spr_star;

static int level_star_count = 0;
static int level_score = 0;
static float level_time = 0;
char level_time_buf[50];

static button btn_space;
static button btn_retry;
static button btn_next;

static cpVect star_1 = {-350,100};
static cpVect star_2 = {0,100};
static cpVect star_3 = {350,100};

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(STATE_ID state_prev)
{
	btn_next->visible = level_star_count != 0;
}

static void pre_update(void)
{
	statesystem_call_update(state_space);
}

static void post_update(void)
{
}

static void draw(void)
{
	//TODO finn ut om spiller d�de eller ikke, istedenfor � bruke level_star_count
	if (level_star_count) {
		draw_color4f(0.3,0.4,0.5,0.5); //draw_color4f(0,0,0,0.8);
		draw_box(4, cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);
		draw_color(COL_WHITE);
		bmfont_center(FONT_SANS, cpv(0, -100),0.8,level_time_buf);
		//bmfont_center(FONT_SANS, cpv(0, 350),1.5,"Level %d.%d cleared!", currentlvl->station,currentlvl->deck);
	} else {
		draw_color4f(0.3,0.1,0.1,0.7); //draw_color4f(0,0,0,0.8);
		draw_box(4, cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);
		draw_color(COL_RED);
		//bmfont_center(FONT_SANS, cpv(0, 350),1.5,"Level failed!", currentlvl->station,currentlvl->deck);
	}

	draw_color4f(1,1,1,1);
	if (level_star_count == 0) draw_color4f(0.1,0.1,0.1,1);
	sprite_render(RLAY_GUI_FRONT, &spr_star, star_1, 0);
	if (level_star_count == 1) draw_color4f(0.1,0.1,0.1,1);
	sprite_render(RLAY_GUI_FRONT, &spr_star, star_2, 0);
	if (level_star_count == 2) draw_color4f(0.1,0.1,0.1,1);
	sprite_render(RLAY_GUI_FRONT, &spr_star, star_3, 0);
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


void leveldone_init(void)
{
	statesystem_register(state_leveldone,0);

	btn_space = button_create(SPRITE_BTN_HOME, 0, "", -GAME_WIDTH/2 + 500, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_retry = button_create(SPRITE_BTN_RETRY, 0, "", 0, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_next = button_create(SPRITE_BTN_NEXT, 0, "", GAME_WIDTH / 2 - 500, -GAME_HEIGHT/2 + 200, 250, 250);

	button_set_click_callback(btn_space, statesystem_set_state, state_stations);
	button_set_click_callback(btn_retry, (btn_click_callback)space_restart_level, 0);
	button_set_click_callback(btn_next, (btn_click_callback)space_next_level, 0);

	button_set_hotkeys(btn_next, KEY_RETURN_1, KEY_RETURN_2);
	button_set_hotkeys(btn_retry, SDL_SCANCODE_SPACE, 0);
	button_set_hotkeys(btn_space, KEY_ESCAPE, SDL_SCANCODE_HOME);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_retry, 1.5);
	button_set_enlargement(btn_next, 1.5);

	state_register_touchable(this, btn_space);
	state_register_touchable(this, btn_retry);
	state_register_touchable(this, btn_next);
	state_register_touchable(this, btn_settings);

	sprite_create(&spr_star,SPRITE_STAR,250,250,0);
}

//TODO create mission objects
void leveldone_status(int stars, int score, float time)
{
	level_star_count = stars;
	level_score = score;
	level_time = (int)(time*1000);

	font_time2str(&level_time_buf[0], time);

	scorelist level_scores;
	level_scores.elements = 0;
	level_scores.filename[0] = '0';
	level_scores.head = NULL;

	char level_score_file[50];
	sprintf(&level_score_file[0], "lvl_%02d_%02d.score", 1, 1);
	highscorelist_readfile(&level_scores,level_score_file);
	highscorelist_addscore(&level_scores, "LVL", level_score, level_time);
	highscorelist_writefile(&level_scores);
	highscorelist_destroy(&level_scores);
}
