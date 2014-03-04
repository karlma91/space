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
#include "../tilemap.h"

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

static Color col_radar = {50,100,150,50};
//static Color col_back = {23*0.8, 93*0.8, 159*0.8, 255*0.8};
static Color col_back = {45,45,45,255*0.90};
static Color col_btns = {255,57,0,255};
static Color col_player = {20,255,20,255};
static Color col_enemy  = {255,20,20,255};

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

	float size = (current_lvl_tmpl->inner_radius / current_lvl_tmpl->outer_radius * MAPSIZE);

	touch_area(start_level,size*2,size*2);
	touch_place(start_level, 0,0);
	button_set_border(start_level,size);
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
	//TODO be able to toggle between radar view and stations view of space station?
	//draw_color(col_back);
	//draw_quad_patch_center(RLAY_BACK_BACK,SPRITE_CIRCLE, box.p, box.s, 50, 0);

	draw_color(COL_WHITE);
	bmfont_center(FONT_SANS, cpv(0,MAPSIZE/2 + 60), 80, from_station->name);

	if (level_loaded) {
		sprite_render_by_id(0, SPRITE_BTN_EDIT, cpv(-GAME_WIDTH/2 + 130, GAME_HEIGHT/2 - 130), cpv(135, 135), 0);
		draw_color(col_back);
		draw_donut(RLAY_BACK_MID, cpv(0,0), current_lvl_tmpl->inner_radius / current_lvl_tmpl->outer_radius * MAPSIZE, MAPSIZE);

		draw_push_matrix();
		float scale = MAPSIZE / current_lvl_tmpl->outer_radius;
		draw_scale(scale, scale);
		space_draw_deck(current_lvl_tmpl);
		tilemap_render(&current_lvl_tmpl->tm);
		llist_begin_loop(current_lvl_tmpl->ll_recipes);
		while(llist_hasnext(current_lvl_tmpl->ll_recipes)) {
			object_recipe * data = llist_next(current_lvl_tmpl->ll_recipes);
			if(data->obj_type == obj_id_player) {
				draw_color(col_player);
			}else{
				draw_color(col_enemy);
			}
			draw_box(0,data->pos,cpv(80,80),cpvtoangle(data->pos),1);
		}
		llist_end_loop(current_lvl_tmpl->ll_recipes);

		draw_pop_matrix();

	} else {
		bmfont_center(FONT_SANS_PLAIN,cpvzero, 60, "Loading!");
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

	edit_level = button_create(SPRITE_BTN2, 1, "", -GAME_WIDTH/2 + 130, GAME_HEIGHT/2 - 130, 135, 135);
	button_set_click_callback(edit_level, button_playedit_callback, state_editor);
	button_set_font(edit_level, FONT_SANS_PLAIN, 1);
	button_set_backcolor(edit_level, col_btns);
	button_set_border(edit_level, 25);
	//button_set_enlargement(edit_level, 2);

	//TODO remove remove_button!
	remove_level =  button_create(0, 0, "X", 0, 600, 30, 30);
	button_set_click_callback(remove_level, button_remove_callback, NULL);
	button_set_enlargement(remove_level, 2); //TODO add confirmation box here!

	start_level =  button_create(SPRITE_BTN2, 1, "Play!", 0, LVLSCREEN_BASE, 370, 135);
	button_set_click_callback(start_level, button_playedit_callback, state_space);
	button_set_backcolor(start_level, col_btns);
	//button_set_enlargement(start_level, 2);
	button_set_font(start_level, FONT_SANS_PLAIN, 1);
	button_set_hotkeys(start_level, SDL_SCANCODE_SPACE, 0);
	button_set_border(start_level, 25);

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
