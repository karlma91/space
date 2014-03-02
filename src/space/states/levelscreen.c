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
#include "editor.h"

static spacelvl *current_lvl_tmpl = NULL;
static station *from_station = NULL;
STATE_ID state_levelscreen;

#define LVLSCREEN_BASE -280
//#define MAX_LEVELS 10
//static button btn_levels[MAX_LEVELS];
static button start_level;
static button remove_level;
static button edit_level;

static view *main_view;

static Color col_radar = {250,230,90,255};

//static Color col_selected= {0,255,0,255};

static button btn_disable;
static button btn_background;

//static int level_count = 0;
static rect box = {{0,0}, {800,800}};
static we_bool level_loaded = WE_TRUE;

static int load_lvl(void *unused)
{
	if (current_lvl_tmpl) {
		spacelvl_free(current_lvl_tmpl);
		current_lvl_tmpl=NULL;
	}
	current_lvl_tmpl = (spacelvl*)spacelvl_parse(WAFFLE_DOCUMENTS, from_station->path);
	level_loaded = WE_TRUE;
	start_level->visible = WE_TRUE;
	edit_level->visible = WE_TRUE;
	return 1;
}


cpVect from, to = {0,0};

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(STATE_ID state_prev)
{
	if (state_prev == state_stations) {
		while (!level_loaded); // wait until previous level has finished loading
		level_loaded = WE_FALSE;
		start_level->visible = WE_FALSE;
		edit_level->visible = WE_FALSE;
		SDL_CreateThread(load_lvl, "Level_loader", NULL);
		from = cpvneg(statesystem_last_touch_view());
		main_view->zoom = 0;
		view_update(main_view, from, 0); //TODO get local touch coordinate
	}
}

static void pre_update(void)
{
	cpVect pos = main_view->p;
	float z_step = 5*dt;
	if (main_view->zoom + z_step < 1) {
		main_view->zoom += z_step;
	} else {
		main_view->zoom = 1;
	}
	pos = cpvlerp(from, to, main_view->zoom);
	if (main_view->zoom) {
		pos = cpvmult(pos, 1 / main_view->zoom);
	}
	touch_place(btn_background, pos.x, pos.y);
	view_update(main_view, pos, 0);
	//TODO use tween to animate both cam pos and size
	//TODO animate view out as well
}

static void post_update(void)
{
}

static void draw(void)
{
	//TODO be able to toggle between radar view and stations view of space station
	draw_color4f(1,1,1,1);
	draw_quad_patch_center(4,SPRITE_DIALOG1, box.p, box.s, 150, 0);
	bmfont_center(FONT_SANS_PLAIN, cpv(0,250), 1.3, from_station->name);

	if (level_loaded) {
		sprite_render_by_id(0, SPRITE_BTN_EDIT, cpv(-260, LVLSCREEN_BASE), cpv(135, 135), 0);
		draw_color(col_radar);
		draw_donut(0, cpv(0,23), current_lvl_tmpl->inner_radius / current_lvl_tmpl->outer_radius * 185, 185);

	} else {
		bmfont_center(FONT_SANS_PLAIN,cpvzero, 1, "Loading!");
	}
}

static int sdl_event(SDL_Event *event) {
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
	if (current_lvl_tmpl) {
		spacelvl_free(current_lvl_tmpl);
		current_lvl_tmpl=NULL;
	}
}

void levelscreen_change_to(station * ship)
{
	from_station = ship;
	statesystem_push_state(state_levelscreen);
}

static void button_playedit_callback(void *state)
{
	while (!level_loaded); //wait until level are loaded
	statesystem_set_state(state);
}

static void button_remove_callback(void *data)
{
	if(waffle_remove(from_station->path) == -1){
		SDL_Log("COLD NOT DELETE FILE %s",from_station->path);
	}
	solarsystem_remove_station(from_station->sol, from_station);
	statesystem_pop_state(NULL);
}

void levelscreen_init(void)
{
	statesystem_register(state_levelscreen,0);

	main_view = state_view_get(state_levelscreen,0);

	edit_level = button_create(SPRITE_BTN1, 1, "", -260, LVLSCREEN_BASE, 135, 135);
	button_set_click_callback(edit_level, button_playedit_callback, state_editor);
	button_set_font(edit_level, FONT_SANS_PLAIN, 1);
	//button_set_enlargement(edit_level, 2);

	//TODO remove remove_button!
	remove_level =  button_create(0, 0, "X", 0, 600, 30, 30);
	button_set_click_callback(remove_level, button_remove_callback, NULL);
	button_set_enlargement(remove_level, 2); //TODO add confirmation box here!

	start_level =  button_create(SPRITE_BTN1, 1, "Play!", 0, LVLSCREEN_BASE, 370, 135);
	button_set_click_callback(start_level, button_playedit_callback, state_space);
	//button_set_enlargement(start_level, 2);
	button_set_font(start_level, FONT_SANS_PLAIN, 1);
	button_set_hotkeys(start_level, SDL_SCANCODE_SPACE, 0);
	state_register_touchable(this, start_level);
	state_register_touchable(this, edit_level);
	state_register_touchable_view(main_view, btn_settings);
	state_register_touchable_view(main_view, remove_level);

	btn_disable = button_create(NULL, 0, "", box.p.x, box.p.y, box.s.x, box.s.y);
	state_register_touchable(this, btn_disable);

	btn_background = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_click_callback(btn_background, statesystem_set_state, state_stations);
	button_set_hotkeys(btn_background, KEY_ESCAPE, 0);
	state_register_touchable(this, btn_background);
}

spacelvl *get_current_lvl_template(void)
{
	return current_lvl_tmpl;
}
