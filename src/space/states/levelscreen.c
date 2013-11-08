/*
 * levelscreen.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include "space.h"
#include "levelscreen.h"

#include "../game.h"
#include "we_defstate.h"

STATE_ID state_levelscreen;

//#define MAX_LEVELS 10
//static button btn_levels[MAX_LEVELS];
static button start_level;

static Color col_default = {255,255,255,255};
static Color col_selected= {0,255,0,255};

static button btn_disable;
static button btn_background;

static int level_count = 0;
static level_ship *current_ship;

static rect box = {{0,0}, {1000,800}};

static float w = 150;
static float h = 150;

static sprite spr_star;
static cpVect star_pos[3] = {{-350,100}, {0,100}, {350,100}};
static int stars_unlocked = 0;

static float alpha = 0;

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
	alpha = 0;
}

static void pre_update(void)
{
}

static void post_update(void)
{
}

static void draw(void)
{
	int i;

	if (alpha < 0.4) {
		alpha += 1 * dt;
		if (alpha > 0.4) {
			alpha = 0.4;
		}
	}

	draw_color4f(0,0,0,alpha);
	draw_box(4, cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);

	draw_color4f(0.1,0.2,0.4,0.6);
	draw_box(3, box.p,box.s,0,1);

	draw_color4f(1,1,1,1);
	bmfont_center(FONT_SANS, cpv(0,box.p.y+box.s.y / 2 - 100), 1.5, current_ship->level_name);

	for (i = 0; i < 3; i++) {
		if (stars_unlocked == i+1) {
			draw_color4f(0,0,0,1);
		}
		sprite_render(RLAY_GUI_FRONT, &spr_star, star_pos[i], 0);
	}
}

static void sdl_event(SDL_Event *event) {
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


void levelscreen_change_to(level_ship * ship)
{
	current_ship = ship;
	level_count = ship->count;
	statesystem_push_state(state_levelscreen);
}

static void button_callback(void *data)
{
	space_start_demo(current_ship->level_name);
}

void levelscreen_init(void)
{
	statesystem_register(state_levelscreen,0);

	Color col_back = {0,51,230,255};

	view *main_view = state_view_get(state_levelscreen,0);

	float y = box.p.y - box.s.y/2 + h / 2 + 20;
	start_level =  button_create(0, 0, "Start", 0, y, 300, 200);
	button_set_callback(start_level, button_callback, NULL);
	button_set_enlargement(start_level, 2);
	button_set_backcolor(start_level, col_back);
	button_set_frontcolor(start_level, col_default);
	button_set_hotkeys(start_level, SDL_SCANCODE_SPACE, 0);
	button_set_font(start_level, FONT_SANS, 2);
	state_register_touchable_view(main_view, start_level);
	state_register_touchable_view(main_view, btn_settings);

	btn_disable = button_create(NULL, 0, "", box.p.x, box.p.y, box.s.x, box.s.y);
	btn_disable->visible = 0;
	state_register_touchable_view(main_view, btn_disable);

	btn_background = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	btn_background->visible = 0;
	button_set_callback(btn_background, statesystem_set_state, state_stations);
	button_set_hotkeys(btn_background, KEY_ESCAPE, 0);
	state_register_touchable_view(main_view, btn_background);

	sprite_create(&spr_star, SPRITE_STAR, 250, 250, 0);
}
