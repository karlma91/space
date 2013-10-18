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

char *txt_buttons[30];

static button *btn_stations;
static button btn_home;

static level_ship *stations;
static int station_count = 2;

static scroll_p scroller;

static int tex_stars;
static int tex_stars_2;

// TODO: cooler system
//tween test
static tween *test;
static tween *tests;
static tween *testr;
static cpVect a, b;
static float r;

// TODO: add this in state struct for each state ?
static layer_system * la_sys;

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
}

static void post_update(void)
{
}

static void sdl_event(SDL_Event *event)
{
}

static void draw(void)
{

	cpVect full = cpv(GAME_WIDTH, GAME_HEIGHT);
	draw_color4f(1,1,1,1);
	float xoffset = scroll_get_xoffset(scroller);
	float yoffset = scroll_get_yoffset(scroller);

	/*{
	float x1 = - (xoffset / 7) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT;
	float y1 = (yoffset / 7) / GAME_HEIGHT;
	float y2 = y1 + 1;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars,cpvzero,map,full,0);
	}
	{
	float x1 = - (xoffset / 4) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT * 2;
	float y1 = (yoffset / 4) / GAME_HEIGHT;
	float y2 = y1 + 1.0 * 2;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars_2,cpvzero,map,full,0);
	}
	{
	float x1 = - (xoffset / 3.6) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT *1;
	float y1 = (yoffset / 3.6) / GAME_HEIGHT;
	float y2 = y1 + 1.0 * 1;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars_2,cpvzero,map,full,0);
	}
	{
	float x1 = - (xoffset / 3.2) / GAME_WIDTH;
	float x2 = x1 + (float) GAME_WIDTH / GAME_HEIGHT * 0.5;
	float y1 = (yoffset / 3.2) / GAME_HEIGHT;
	float y2 = y1 + 1 * 0.5;
	float map[8] = {x1, y2,
					x2, y2,
					x1, y1,
					x2, y1};
	draw_texture(tex_stars_2,cpvzero,map,full,0);
	}*/


	draw_color4f(1,1,1,1);
	layersystem_render(la_sys, current_view);
	int i;
	for (i = 0; i < station_count; i++) {
		touch_place(btn_stations[i], -(station_count - 1) / 2.0 * 650 + 1000 * i + xoffset, yoffset+(i-0.5)*270);
		btn_stations[i]->calls->render(btn_stations[i]);
		//TODO tegne minste antall stjerner av levlene i romstasjonen under den?
		//TODO skrive antall levler klart?
	}


	draw_box(cpvadd(a,cpv(xoffset,yoffset)), b, r, 1);

	//cpVect pos = {0,0.7f*GAME_HEIGHT/2};
	//draw_texture(tex_title, &pos, TEX_MAP_FULL, 1200, 300, 0);

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

static void open_upgrades(void)
{
	statesystem_push_state(state_store);
}

void stations_init(void)
{
	statesystem_register(state_stations, 0);

	level_get_ships(&stations, &station_count);

	btn_stations = calloc(station_count, sizeof(button));

	Color col_back = {255,154,127,255};
	//Color col_text = {1,1,1,1};

	btn_home = button_create(SPRITE_PLAYER, 0, "", 0, 0, 250, 250);
	button_set_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 2);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	state_register_touchable(this, btn_home);

	int i;
	for (i = 0; i < station_count; i++) {
		char stri[10];
		sprintf(stri, "%d", i+1);
		float size = 350 + (i ? -50 : 50);
		btn_stations[i] = button_create(SPRITE_STATION_01, 0, "", -1,-1, size, size);
		button_set_callback(btn_stations[i], button_callback, &stations[i]);
		button_set_backcolor(btn_stations[i], col_back);
		button_set_animated(btn_stations[i], 1, (i ? 18 : 15));
		button_set_enlargement(btn_stations[i], 1.5);
		button_set_hotkeys(btn_stations[i], digit2scancode[(i+1) % 10], 0);
		btn_stations[i]->visible = 0;

		state_register_touchable(this, btn_stations[i]);
	}

	state_register_touchable(this, btn_settings);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000); // max 4 000 gu / sec
	state_register_touchable(this, scroller);

	tex_stars = texture_load("stars.jpg");
	tex_stars_2 = texture_load("stars_2.png");

	la_sys = layersystem_new();
	for(i = 0; i<la_sys->num_layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
		float f = (la_sys->num_layers - i * 0.9f) / (la_sys->num_layers);
		layersystem_set_layer_parallax(la_sys, i, f, 1);
	}
	for(i = 0; i<100; i++){
		int layer = roundf(we_randf*(la_sys->num_layers-1));
		float size = 150 + we_randf*70 - layer*1.5;
		cpVect pos = cpvmult(cpv(we_randf-0.5,we_randf-0.5),2600);
		layersystem_add_sprite(la_sys, layer, SPRITE_SPIKEBALL, size, size, pos, we_randf*WE_2PI);
	}
}

