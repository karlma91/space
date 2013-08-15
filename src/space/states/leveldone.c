/*
 * leveldone.c
 *
 *  Created on: 11. juli 2013
 *      Author: Mathias
 */
#include <stdio.h>

#include "leveldone.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"

#include "../../engine/graphics/draw.h"


#include "../../engine/input/button.h"
#include "../../engine/data/highscorelist.h"

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

static void on_enter()
{
}

static void pre_update()
{
	statesystem_call_update(state_space);
}

static void post_update()
{
}

static void draw()
{
	draw_color4f(0,0,0,0.8);
	draw_box(0,0,GAME_WIDTH,GAME_HEIGHT,0,1);

	//TODO finn ut om spiller d�de eller ikke, istedenfor � bruke level_star_count
	if (level_star_count) {
		draw_color4f(1,1,1,1);
		setTextAlign(TEXT_CENTER);
		setTextSize(60);
		font_drawText(0,-100, level_time_buf);

		draw_color4f(1,1,1,1);
		setTextAlign(TEXT_CENTER);
		setTextSize(70);
		font_drawText(0,350, "LEVEL CLEARED");
	} else {
		draw_color4f(1,0,0,1);
		setTextAlign(TEXT_CENTER);
		setTextSize(70);
		font_drawText(0,350, "LEVEL FAILED");
	}

	draw_color4f(1,1,1,1);
	if (level_star_count == 0) draw_color4f(0.2,0.2,0.2,1);
	sprite_render(&spr_star, &star_1, 0);
	if (level_star_count == 1) draw_color4f(0.2,0.2,0.2,1);
	sprite_render(&spr_star, &star_2, 0);
	if (level_star_count == 2) draw_color4f(0.2,0.2,0.2,1);
	sprite_render(&spr_star, &star_3, 0);
}

static void sdl_event(SDL_Event *event)
{
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


void leveldone_init()
{
	statesystem_register(state_leveldone,0);

	btn_space = button_create(SPRITE_HOME, 0, "", -GAME_WIDTH/2 + 500, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_retry = button_create(SPRITE_RETRY, 0, "", 0, -GAME_HEIGHT/2 + 200, 250, 250);
	btn_next = button_create(SPRITE_NEXT, 0, "", GAME_WIDTH / 2 - 500, -GAME_HEIGHT/2 + 200, 250, 250);

	button_set_callback(btn_space, statesystem_set_state, state_stations);
	button_set_callback(btn_retry, space_restart_level, 0);
	button_set_callback(btn_next, space_next_level, 0);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_retry, 1.5);
	button_set_enlargement(btn_next, 1.5);

	statesystem_register_touchable(this, btn_space);
	statesystem_register_touchable(this, btn_retry);
	statesystem_register_touchable(this, btn_next);
	statesystem_register_touchable(this, btn_settings);

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
	sprintf(&level_score_file[0], "lvl_%02d_%02d.score", currentlvl->station, currentlvl->deck);
	highscorelist_readfile(&level_scores,level_score_file);
	highscorelist_addscore(&level_scores, "LVL", level_score, level_time);
	highscorelist_writefile(&level_scores);
}
