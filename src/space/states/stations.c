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


	bmfont_center(FONT_SANS, cpv(0,500),1.5,"SPACE");
	int i;
	for(i = 0; i<station_count; i++) {
		bmfont_center(FONT_SANS, stations[i].pos,1, stations[i].level_name);
	}

	bmfont_center(FONT_SANS, cpv(1400,-1150),1,"CREDTIS:\nMathias Wilhelmsen\nKarl Magnus Kalvik");
	draw_color4b(200,210,230,255);
	bmfont_right(FONT_SANS, cpv(-600,300),1,"the quick brown fox jumps over the lazy dog\nTHE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");


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

	btn_home = button_create(SPRITE_PLAYERGUN001, 0, "", 0, 0, 250, 250);
	button_set_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 2);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	state_register_touchable_view(main_view, btn_home);

	btn_editor = button_create(SPRITE_WRENCH, 0, "", 0, -600, 873/2, 247/2);
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

	state_add_layers(state_stations, 10);

	int layers = state_layer_count(state_stations);

	for(i = 2; i<layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
		float f = (layers - i * 0.99f) / (layers);
		state_set_layer_parallax(state_stations, i, f, f);
	}

	for(i = 0; i<1000; i++){
		int layer =  2 + roundf(we_randf*(layers-1-2));
		float size = 150 + we_randf*90 - layer*4;
		cpVect pos = cpvmult(cpv(we_randf-0.5,we_randf-0.5),6600);
		SPRITE_ID spr;
		int s = rand() & 7;
		switch(s) {
        default: spr = SPRITE_SPIKEBALL; break;
		case 1: spr = SPRITE_GEAR; break;
		case 2: spr = SPRITE_STATION001; break;
		case 3: spr = SPRITE_TANKWHEEL001; break;
		}

		state_add_sprite(state_stations, layer, spr, size, size, pos, we_randf*WE_2PI * (spr != SPRITE_STATION001));
	}
}

