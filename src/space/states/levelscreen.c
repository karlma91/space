/*
 * levelscreen.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include <stdio.h>
#include "space.h"
#include "levelscreen.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/state.h"
#include "../../engine/graphics/font.h"
#include "../../engine/graphics/draw.h"

STATE_ID state_levelscreen;

#define MAX_LEVELS 10
static button btn_levels[MAX_LEVELS];

static Color col_default = {1,1,1,1};
static Color col_selected= {0,1,0,1};

static button btn_disable;
static button btn_background;

static int selected_level = 0;
static int level_count = 0;
static level_ship *current_ship;

static rect box = {0,-100,1000,700};

static float w = 150;
static float h = 150;

static char title[40];

static sprite spr_star;

static cpVect star_pos[3] = {{-350,0}, {0,0}, {350,0}};
static cpVect star_1 = {-350,0};
static cpVect star_2 = {0,0};
static cpVect star_3 = {350,0};

static int stars_unlocked = 0;

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
	int i;

	draw_color4f(0.1,0.2,0.4,0.6);
	draw_box(box.x,box.y,box.w,box.h,0,1);

	draw_color4f(1,1,1,1);
	setTextAlign(TEXT_CENTER);
	setTextSize(50);
	font_drawText(0,box.y+box.h / 2 - 60,title);

	for (i = 0; i < 3; i++) {
		if (stars_unlocked == i+1) {
			draw_color4f(0,0,0,1);
		}
		sprite_render(&spr_star, &star_pos[i], 0);
	}
}

static void sdl_event(SDL_Event *event) {
}

static void on_leave()
{
}

static void destroy()
{
}

static void set_selected_level(int level)
{
	if (selected_level > 0 && selected_level <= level_count) {
		button_set_frontcolor(btn_levels[selected_level-1], col_default);
	}
	button_set_frontcolor(btn_levels[level-1], col_selected);

	selected_level = level;
	sprintf(&title[0], "LEVEL %d", level);
	//TODO update star score info + best time
	stars_unlocked = 3 - ((level & 3) ^ 0x1);//TMP TODO store stars unlocked in level/station struct
}

void levelscreen_change_to(level_ship * ship)
{
	current_ship = ship;
	level_count = ship->count;
	set_selected_level(1); //TODO select last unlocked level

	float margin = 30;
	float y = box.y - box.h/2 + h / 2 + margin;
	int i;
	for (i = 0; i < level_count; i++) {
		float x = box.x - box.w/2 + (box.w-margin*2) * (i+0.5) / level_count;
		button_set_position(btn_levels[i], x, y);
		button_set_visibility(btn_levels[i], 1);
	}
	for (;i < MAX_LEVELS; i++) {
		button_set_visibility(btn_levels[i], 0);
	}

	statesystem_push_state(state_levelscreen);
}

static void button_callback(void *data)
{
	int level = *((int *) &data);

	if (level == selected_level) {
		space_start_demo();
		space_init_level(current_ship->id, level);
	} else {
		set_selected_level(level);
	}
}

void levelscreen_init()
{
	statesystem_register(state_levelscreen,0);

	Color col_back = {0,0.2,0.9,1};

	int i;
	for (i = 0; i < MAX_LEVELS; i++) {
		char stri[MAX_LEVELS];
		sprintf(stri, "%d", i+1);

		button btn = button_create(0, 0, stri, 0, 0, w, h);
		btn_levels[i] = btn;

		button_set_callback(btn, button_callback, (NULL + i+1));
		button_set_enlargement(btn, 2);
		button_set_backcolor(btn, col_back);
		button_set_frontcolor(btn, col_default);
		statesystem_register_touchable(state_levelscreen, btn);
	}

	btn_disable = button_create(NULL, 0, "", box.x, box.y, box.w, box.h);
	statesystem_register_touchable(this, btn_disable);

	btn_background = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_callback(btn_background, statesystem_set_state, state_stations);
	statesystem_register_touchable(this, btn_background);

	sprite_create(&spr_star, SPRITE_STAR, 250, 250, 0);
}
