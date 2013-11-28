/*
 * stations.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include "../game.h"
#include "we_defstate.h"
#include "levelscreen.h"

STATE_ID state_stations;

static SPRITE_ID spr_sun;

static button *btn_stations;
static button btn_home;
static button btn_editor;

static level_ship *stations;
static int station_count = 2;

static scroll_p scroller;

// TODO: cooler system
//tween test
static tween *test;
static tween *tests;
static tween *testr;
static cpVect a, b;
static float r;

static view *main_view;

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
    //TODO: TMP TWEEN TEST
    test = tween_cpv_create(cpv(0,300), cpv(500,500), 2, LinearInterpolation);
    tests = tween_cpv_create(cpv(50,50), cpv(100,100), 2, ElasticEaseInOut);
    tween_repeat(tests, 1, 1);
    float str = 0;
    float endr = 3.14;
    testr = tween_create(&(str), &(endr), 1, 3, ElasticEaseInOut);
    tween_repeat(testr, 2, 1);
}

static void pre_update(void)
{
    test = tween_cpv_is_done_remove(test, &a);
    tests = tween_cpv_is_done_remove(tests, &b);
    testr = tween_float_is_done_remove(testr, &r);

    main_view->zoom = scroll_get_zoom(scroller);
    float rot = scroll_get_rotation(scroller);
	cpVect offset = cpvneg(scroll_get_offset(scroller));
	view_update(main_view, offset, rot);
}

static void post_update(void)
{
}

static int sdl_event(SDL_Event *event)
{
	return 0;
}

static void draw(void)
{
	draw_color4f(1,1,1,1);
	draw_box(1, a, b, r, 1);

	//bmfont_center(FONT_SANS, cpv(0,500),1.5,"SPACE");
	int i;
	for(i = 0; i<station_count; i++) {
		bmfont_center(FONT_SANS, stations[i].pos,1, stations[i].level_name);
	}

	bmfont_center(FONT_SANS, cpv(1400,-1150),1,"CREDITS:\nMathias Wilhelmsen\nKarl Magnus Kalvik");
	//draw_color4b(200,210,230,255);
	//bmfont_right(FONT_SANS, cpv(-600,300),1,"the quick brown fox jumps over the lazy dog\nTHE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");

	float f = 1/2.0;
	float size_dither = (engine_time*f - floor(engine_time*f) - 0.5)*(engine_time*f - floor(engine_time*f) - 0.5)*10;
	cpVect sun_size = {400+size_dither,400+size_dither};
	static float a_sun_add1 = 0;
	static float a_sun_add2 = 0;
    float spd = 0.01;
	a_sun_add1 += dt*WE_2PI*spd;
	a_sun_add2 -= dt*WE_2PI*spd;

	//draw_color4b(100,100,100,0);
	//sprite_render_index_by_id(RLAY_GAME_BACK, SPRITE_GLOW, 0, cpvzero, cpvmult(sun_size,1.5), a_sun_base);
	draw_color4b(255,40,20,255);
	sprite_render_index_by_id(RLAY_GAME_BACK, spr_sun, 0, cpvzero, sun_size, 0);
	draw_color4b(255,192,30,0);
	sprite_render_index_by_id(RLAY_GAME_BACK, spr_sun, 1, cpvzero, sun_size, a_sun_add1);
	draw_color4b(255,132,30,0);
	sprite_render_index_by_id(RLAY_GAME_BACK, spr_sun, 2, cpvzero, sun_size, a_sun_add2);
}

static void button_callback(void *data)
{
	levelscreen_change_to(data);
}

static void on_pause(void)
{
}

static void on_leave(void)
{
    test = tween_release(test);
    tests = tween_release(tests);
    testr = tween_release(testr);
}

static void destroy(void)
{
}

static void open_upgrades(void *unused)
{
	statesystem_push_state(state_store);
}
static void open_editor(void *unused)
{
	statesystem_set_state(state_editor);
}

void stations_init(void)
{
	statesystem_register(state_stations, 0);

	level_get_ships(&stations, &station_count);

	btn_stations = calloc(station_count, sizeof(button));

	Color col_back = {255,255,255,255};
	//Color col_text = {1,1,1,1};

	main_view = state_view_get(state_stations, 0);

	spr_sun = sprite_link("sun01");
	btn_home = button_create(SPRITE_PLAYERBODY001, 0, "", 800, 600, 250, 250);
	button_set_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 2);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	state_register_touchable(this, btn_home);

	btn_editor = button_create(SPRITE_WRENCH, 0, "", -440, 840, 873/2, 247/2);
	button_set_callback(btn_editor, open_editor, 0);
	button_set_enlargement(btn_editor, 2);
	button_set_hotkeys(btn_editor, SDL_SCANCODE_E, -1);
	state_register_touchable(this, btn_editor);

	int i;
	for (i = 0; i < station_count; i++) {
		char stri[10];
		sprintf(stri, "%d", i+1);
		float size = 350 + (i ? -50 : 50);
		btn_stations[i] = button_create(SPRITE_STATION001, 0, "", stations[i].pos.x,stations[i].pos.y, size, size);
		button_set_callback(btn_stations[i], button_callback, &stations[i]);
		button_set_backcolor(btn_stations[i], col_back);
		button_set_animated(btn_stations[i], 1, (i ? 18 : 15));
		button_set_enlargement(btn_stations[i], 1.5);
		button_set_hotkeys(btn_stations[i], digit2scancode[(i+1) % 10], 0);

		state_register_touchable(this, btn_stations[i]);
	}

	state_register_touchable_view(main_view, btn_settings);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000, 1, 1, 0); // max 4 000 gu / sec
	scroll_set_bounds(scroller, cpBBNew(-GAME_WIDTH-200, -GAME_HEIGHT-200, GAME_WIDTH+200, GAME_HEIGHT+200));
	state_register_touchable_view(main_view, scroller);

	state_add_layers(state_stations, 22);

	int layers = state_layer_count(state_stations);

	for(i = 11; i<layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
		float f = (layers - i * 0.99f) / (layers);
		state_set_layer_parallax(state_stations, i, f, f);
	}

	SPRITE_ID spr = sprite_link("starcross01");
    Color col1 = {255,255,255,0};
    Color col2 = {0,128,0,0};
	for(i = 0; i < 2000; i++){
		int layer =  11 + roundf((1-we_randf*we_randf*we_randf)*(layers-1-11));
		float size = 250 + we_randf*90 - layer*4;
		//byte l = 255 - 200 * layer / layers;
		//col = {l,l,l,255};
		float rand_x = we_randf;
		float rand_y = we_randf;
		cpVect pos = cpvmult(cpv(rand_x-0.5,rand_y-0.5),30000);
		float f = minf(rand_x*0.6 + 0.4*rand_y*(0.9+0.1*rand_x), 1);
		col2.r = 255*(f);
		col2.b = 255*(1-f);
		if (we_randf < 0.1) {
			state_add_sprite(state_stations, layer, SPRITE_SPIKEBALL, size,size, pos, 0, col2);
		} else {
			state_add_dualsprite(state_stations, layer, spr, pos, cpv(size,size), col1, col2);
		}
	}
}

