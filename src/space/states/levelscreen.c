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
#include "../../engine/tween/tween.h"

static spacelvl *current_lvl_tmpl = NULL;
static station *from_station = NULL;
STATE_ID state_levelscreen;

static button start_level;
static button remove_level;
static button edit_level;

static view *main_view;

//static Color col_radar = {50,100,150,50};
//static Color col_back = {23*0.8, 93*0.8, 159*0.8, 255*0.8};
static Color col_back = {45,45,45,255*0.90};
static Color col_btns = {255,57,0,255};
static Color col_player = {20,255,20,255};
static Color col_enemy  = {255,20,20,255};

static button btn_background;
static we_bool level_loaded = WE_TRUE;
static tween * out;
extern view *station_view;

enum {
	FADE_DONE,
	FADE_IN,
	FADE_OUT
};
static int fading = FADE_IN;

static int load_lvl(void *unused)
{
	if (current_lvl_tmpl) {
		spacelvl_free(current_lvl_tmpl);
		current_lvl_tmpl=NULL;
	}
	current_lvl_tmpl = (spacelvl*)spacelvl_parse(WAFFLE_DOCUMENTS, from_station->path);

	//int i = 1<<28; while(--i); //TMP: simulate lag

	start_level->visible = WE_TRUE;
	float size = (current_lvl_tmpl->inner_radius / current_lvl_tmpl->outer_radius * MAPSIZE);
	size = fmaxf(size, 90);
	touch_area(start_level,size*2,size*2);
	touch_place(start_level, 0,0);
	button_set_border(start_level,size);
	edit_level->visible = WE_TRUE;

	level_loaded = WE_TRUE;
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
		from = cpvneg(view_world2view(station_view,from_station->pos));
		main_view->zoom = 0;
		main_view->p = from;
		fading = FADE_IN;

		tween_system_clear(current_tween_system);
		tween * t = tween_system_new_tween(current_tween_system, cpvect_accessor, &main_view->p,0.5);
		tween_target(t,TWEEN_FALSE, 0.0, 0.0);
		tween_easing(t,QuinticEaseOut);
		t = tween_system_new_tween(current_tween_system, float_accessor, &main_view->zoom,0.5);
		tween_target(t,TWEEN_FALSE, 1.0);
		tween_easing(t,QuinticEaseOut);

	}
}

static void pre_update(void)
{
	touch_place(btn_background, main_view->p.x, main_view->p.y);
}

static void post_update(void)
{
	if (main_view->zoom > 0 && main_view->zoom < 0.99) {
		main_view->p = cpvmult(main_view->p, 1 / main_view->zoom);
	}
	if( fading == FADE_OUT) {
		cpVect p = cpvneg(view_world2view(station_view,from_station->pos));
		tween_target(out,TWEEN_FALSE,p.x,p.y);
	}
}

static void draw(void)
{
	draw_color(COL_WHITE);
	bmfont_center(FONT_SANS, cpv(0,MAPSIZE/2 + 60), 80, from_station->name);

	if (level_loaded) {
		sprite_render_by_id(0, SPRITE_BTN_EDIT, cpv(-GAME_WIDTH/2 + 130, GAME_HEIGHT/2 - 130), cpv(135, 135), 0);
		draw_color(col_back);
		draw_donut(RLAY_BACK_MID, cpv(0,0), current_lvl_tmpl->inner_radius / current_lvl_tmpl->outer_radius * MAPSIZE, MAPSIZE);

		draw_push_matrix();
		float scale = MAPSIZE / current_lvl_tmpl->outer_radius;
		draw_scale(scale, scale);
		//space_draw_deck(current_lvl_tmpl);
		tilemap_render(&current_lvl_tmpl->tm);
		llist_begin_loop(current_lvl_tmpl->ll_recipes);
		while (llist_hasnext(current_lvl_tmpl->ll_recipes)) {
			object_recipe * data = llist_next(current_lvl_tmpl->ll_recipes);
			draw_color((data->obj_type == obj_id_player) ? col_player : col_enemy);
			draw_box(3, data->pos, cpv(80, 80), cpvtoangle(data->pos), 1);
		}
		llist_end_loop(current_lvl_tmpl->ll_recipes);
		draw_pop_matrix();

		draw_color4b(200,200,200,200);
		GLfloat triangle[8] = {
				-25,-44,
				 50,  0,
				-25, 44,
				-25, 44};

		float subimg[8];
		texture_bind_virt(sprite_get_texture(SPRITE_WHITE));
		sprite_get_subimg_by_index(SPRITE_WHITE,0,subimg);
		sprite_subimg tex = sprite_get_subimg(SPRITE_WHITE);
		draw_quad_new(0, triangle, &tex.x1);
	} else {
		bmfont_center(FONT_SANS, cpvzero, 60, "Loading!");
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
	fading = FADE_DONE;
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

void tween_fadeout(tween_instance * t, void *userdata)
{
	statesystem_pop_state(NULL);
}

static void fadeout(void *unused)
{
	if(fading == FADE_IN){
		fading = FADE_OUT;
		out = tween_system_new_tween(current_tween_system, cpvect_accessor, &main_view->p,0.3);
		tween_target(out,TWEEN_FALSE, 0.0, 0.0);
		tween_easing(out,LinearInterpolation);
		tween * t = tween_system_new_tween(current_tween_system, float_accessor, &main_view->zoom, 0.3);
		tween_target(t,TWEEN_FALSE, 0.0);
		tween_set_callback(t, tween_fadeout, NULL);
		tween_easing(t,LinearInterpolation);
	}
}

void levelscreen_init(void)
{
	statesystem_register(state_levelscreen, 0);
	state_enable_tween_system(this, 1);
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

	start_level =  button_create(SPRITE_BTN2, 1, "", 0,0, 370, 135);
	button_set_click_callback(start_level, button_playedit_callback, state_space);
	button_set_backcolor(start_level, col_btns);
	button_set_enlargement(start_level, 1.1);
	button_set_hotkeys(start_level, SDL_SCANCODE_SPACE, 0);
	button_set_border(start_level, 25);

	state_register_touchable(this, start_level);
	state_register_touchable(this, edit_level);
	state_register_touchable_view(main_view, btn_settings);
	state_register_touchable_view(main_view, remove_level);

	btn_background = button_create(NULL, 0, "", 0, 0, GAME_WIDTH, GAME_HEIGHT);
	button_set_click_callback(btn_background, fadeout, NULL);
	button_set_hotkeys(btn_background, KEY_ESCAPE, 0);
	state_register_touchable(this, btn_background);
}

spacelvl *get_current_lvl_template(void)
{
	return current_lvl_tmpl;
}
