/*
 * stations.c
 *
 *  Created on: 11. juli 2013
 *      Author: Karl
 */

#include "../game.h"
#include "we_defstate.h"
#include "levelscreen.h"
#include "../solarsystem.h"
#include "../../engine/tween/tween.h"

STATE_ID state_stations;

static button btn_home;
LList user_system;

static scroll_p scroller;

static Color col_unfocus = {0,0,0,0}; //{0,0,0,255};
static we_bool lost_focus = WE_FALSE;
static float focus_a = 0;

// TODO: cooler system
//tween test
static tween *cpv_test, *float_test;
static sprite s;
static sprite_ori ori;
static Color c;
static tween *sprite_tw, *color_tw;
static cpVect a;
static float r;

view *station_view;

/*
enum {
	A,
	C,
	E,
	NOTE_COUNT
};
static Mix_Chunk *note[NOTE_COUNT];

static int tone_index1 = 0, tone_index2 = 0, tone_index3 = 0;
#define TONE_COUNT 8

typedef struct tone {
	int note, ms;
} tone;

static tone score1[TONE_COUNT] = {
		{A, 250},
		{C, 250},
		{A, 250},
		{C, 250},
		{A, 250},
		{C, 250},
		{A, 250},
		{C, 250}
};
static tone score2[TONE_COUNT] = {
		{A, 250},
		{C, 250},
		{E, 250},
		{A, 250},
		{C, 250},
		{E, 250},
		{C, 250},
		{E, 250}
};
static tone score3[TONE_COUNT] = {
		{E, 250},
		{A, 250},
		{C, 250},
		{A, 250},
		{E, 250},
		{A, 250},
		{C, 250},
		{A, 250}
};
*/
void loop_end(int channel)
{
	/*
	switch(channel) {
	case 0:
		Mix_PlayChannelTimed(channel, note[score1[tone_index1].note], -1, score1[tone_index1].ms);
		if (++tone_index1 >= TONE_COUNT) tone_index1 = 0;
		break;
	case 1:
		Mix_PlayChannelTimed(channel, note[score2[tone_index2].note], -1, score2[tone_index2].ms);
		if (++tone_index2 >= TONE_COUNT) tone_index2 = 0;
		break;
	case 2:
		Mix_PlayChannelTimed(channel, note[score3[tone_index3].note], -1, score3[tone_index3].ms);
		if (++tone_index3 >= TONE_COUNT) tone_index3 = 0;
		break;
	}
	*/
}

void sound_testing(void)
{
	/*
	note[A] = sound_loadchunk("sineA3.ogg");
	note[C] = sound_loadchunk("sineC4.ogg");
	note[E] = sound_loadchunk("sineE4.ogg");
	Mix_GroupChannels(0,2, 1);
	Mix_FadeInChannelTimed(0, note[A], -1, 100, 5000);
	Mix_FadeInChannelTimed(1, note[C], -1, 100, 5000);
	Mix_FadeInChannelTimed(2, note[E], -1, 100, 5000);
	//Mix_FadeInChannelTimed(2, toneE, -1, 0, -1);
	Mix_ChannelFinished(loop_end);
	//Mix_ //TODO check out callback
	*/
}


/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(STATE_ID state_prev)
{
    //TODO use tween for colors here!
    lost_focus = WE_FALSE;
}

static void pre_update(void)
{
	// TWEEN tests
	tween_tween_update(cpv_test,dt);
	tween_tween_update(float_test,dt);
	tween_tween_update(sprite_tw,dt);
	tween_tween_update(color_tw,dt);

    station_view->zoom = scroll_get_zoom(scroller);
    float rot = scroll_get_rotation(scroller);
	cpVect offset = cpvneg(scroll_get_offset(scroller));
	view_update(station_view, offset, rot);

	llist_begin_loop(user_system);
	while (llist_hasnext(user_system)) {
		solarsystem *sol = (solarsystem *)llist_next(user_system);
		solarsystem_update(sol);
	}
	llist_end_loop(user_system);
}

static void post_update(void)
{
}

static int sdl_event(SDL_Event *event)
{
	float z = scroll_get_zoom(scroller);
	switch (event->type) {
	case SDL_MOUSEWHEEL:
		z *=  event->wheel.y > 0 ? 1.1 : 0.9;
		scroll_set_zoom(scroller, z);
		return 1;
	}
	return 0;
}

static void draw(void)
{
	draw_color4f(1,1,1,1);
	//draw_box(1, a, b, r, 1);

	draw_push_matrix();
	draw_translate(1900,-1750);
	draw_rotate(-current_view->rotation);

	draw_color(c);
	sprite_render_ori(0,&s,&ori);
	draw_color4f(1,1,1,1);

	draw_push_matrix();
	draw_translatev(a);
	draw_rotate(r);
	bmfont_center(FONT_SANS_PLAIN, cpvzero,100,"SPACEW");
	draw_pop_matrix();

	bmfont_center(FONT_SANS_PLAIN, cpv(100,10000),100,"Space (working title)\nETA: 25. Jan 2014\n\nCredits:\nMathias Wilhelmsen\nKarl Magnus Kalvik\n\nAlpha Testers\nJacob & Jonathan Høgset [iPod 4th]\nBård-Kristian Krohg [iPod 3rd]");
	bmfont_center(FONT_SANS, cpvzero,100,"Font sans");
	bmfont_center(FONT_BIG, cpv(0,100),100,"Font big");
	bmfont_center(FONT_COURIER,  cpv(0,200),100,"Font courier");
	bmfont_center(FONT_NORMAL,  cpv(0,300),100,"Font normal");
	bmfont_center(FONT_SANS_PLAIN,  cpv(0,400),100,"Font sans plain");
	draw_pop_matrix();


	llist_begin_loop(user_system);
	cpVect p;
	int i = 0;
	while (llist_hasnext(user_system)) {
		solarsystem *sol = (solarsystem *)llist_next(user_system);
		solarsystem_update(sol);
		solarsystem_draw(sol);
		if(i>0){
			draw_color4b(20,20,80,50);
			draw_quad_line(RLAY_BACK_MID,p, sol->origo, 6/current_view->zoom);
		}
		p = sol->origo;
		i++;
	}
	llist_end_loop(user_system);
}

static void draw_gui(view *v)
{
#ifdef GOT_SDL_NET
	draw_color4f(!db_connected,db_connected,0,1);
	draw_circle(0, cpv(-v->view_width/2+20, v->view_height/2-20), 15);
#endif
	draw_color4f(1,1,1,1);

	float step = 4*dt;
	focus_a = lost_focus ?
			(focus_a + step < 1 ? focus_a + step : 1):
			(focus_a - step > 0 ? focus_a - step : 0);
	Color col = col_unfocus;
	col.r *= focus_a, col.g *= focus_a, col.b *=focus_a, col.a *=focus_a;
	draw_color(col);
	draw_box(0, cpvzero,cpv(GAME_WIDTH,GAME_HEIGHT),0,1);
}

static void on_pause(void)
{
}

static void on_leave(STATE_ID state_next)
{
    lost_focus = WE_TRUE;
}

static void destroy(void)
{
}

static void open_upgrades(void *unused)
{
	statesystem_push_state(state_store);
}

static void tween_callback_test(void *data, void *userdata)
{
	tween_tween_reset(sprite_tw);
	tween_tween_set_start(sprite_tw);
	tween_target(sprite_tw,1,500 - we_randf*1000,500 - we_randf*1000, WE_PI-we_randf*2*WE_PI, 0.05 - we_randf * 0.08);
}

static void tween_color_test(void *data, void *userdata)
{
	tween_tween_reset(color_tw);
	tween_tween_set_start(color_tw);
	tween_target(color_tw,0,we_randf*255,we_randf*255, we_randf*255);
}


void stations_init(void)
{
	srand(0x9b3a09fa);
	statesystem_register(state_stations, 0);

	a = cpv(0,5000);
	cpv_test = tween_new_tween(cpvect_accessor, &a, 1.8);
	tween_target(cpv_test,1, 2500 - we_randf*5000, 2500 - we_randf*5000);
	tween_easing(cpv_test,ElasticEaseInOut);
	tween_repeat(cpv_test,1,100,1);

	r = 0;
	float_test = tween_new_tween(float_accessor, &r, 4.5);
	tween_easing(float_test,ElasticEaseInOut);
	tween_repeat(float_test,1,100,1);

	sprite_create(&s,SPRITE_GEAR,400,400,0.4);
	ori.p = cpv(-5000,400);
	ori.angle = 0;
	ori.size = 1;
	sprite_tw = tween_new_tween(sprite_accessor, &ori, 0.15);
	tween_target(sprite_tw, 0,   -4000.0, 1000.0, 2.0*WE_PI, 2.0);
	tween_easing(sprite_tw,LinearInterpolation);
	tween_set_callback(sprite_tw, tween_callback_test, NULL);

	c = color_new3b(255,255,0);
	color_tw = tween_new_tween(color_accessor, &c, 1.5);
	tween_target(color_tw,0, 10.0,255.0,250.0);
	tween_easing(color_tw, LinearInterpolation);
	tween_set_callback(color_tw, tween_color_test, NULL);

	user_system = solarsystem_load_solar("levels/userlevels.json");

	Color col_back = {30,100,200,255};

	station_view = state_view_get(state_stations, 0);
	station_view->GUI = draw_gui;

	btn_home = button_create(SPRITE_BTN2, 1, "Upgrade", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 100, 350, 125);
    button_set_border(btn_home, 25);
	button_set_click_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 0.9);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	button_set_backcolor(btn_home, col_back);
	state_register_touchable_view(station_view, btn_home);

	llist_begin_loop(user_system);
	while (llist_hasnext(user_system)) {
		solarsystem_register_touch((solarsystem *)llist_next(user_system), this);
	}
	llist_end_loop(user_system);

	state_register_touchable_view(station_view, btn_settings);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000, 1, 1, 0); // max 4 000 gu / sec
	scroll_set_bounds(scroller, cpBBNew(-60000, -60000, 60000, 60000));
	scroll_set_zoomlimit(scroller, 0.01, 1);
	scroll_set_zoom(scroller,0.2);
	state_register_touchable_view(station_view, scroller);

	state_add_layers(state_stations, 22);

	int layers = state_layer_count(state_stations);

	int i;
	for(i = 11; i<layers; i++){
		float f = (layers - i * 0.99f) / (layers);
		state_set_layer_parallax(state_stations, i, f, f);
	}

	SPRITE_ID spr = sprite_link("starcross01");
	for(i = 0; i < 2000; i++){
        Color col1 = {255,255,255,0};
        Color col2 = {0,0,0,0};
		int layer =  11 + roundf((1-we_randf*we_randf)*(layers-1-11));
		float size = 850 + we_randf*190 - layer*4;
		//byte l = 255 - 200 * layer / layers;
		//col = {l,l,l,255};
		float rand_x = we_randf;
		float rand_y = we_randf;
		cpVect pos = cpvmult(cpv(rand_x-0.5,rand_y-0.5),400000);
		float f = minf(rand_x*0.6 + 0.4*rand_y*(0.9+0.1*rand_x), 1);
		//col2.g = 255*(f);
		//col2.b = 255*(1-f);
        col2 = draw_col_rainbow((f+0.5)*1536);
		if (we_randf < 0.1) {
			col2.a = 1;
			state_add_sprite(state_stations, layer, SPRITE_SPIKEBALL, size,size, pos, 0, col2);
		} else {
			col2.r = (255 + col2.r)/2;
			col2.g = (255 + col2.g)/2;
			col2.b = (255 + col2.b)/2;
			col2.a = 0;
			state_add_dualsprite(state_stations, layer, spr, pos, cpv(size,size), col1, col2);
		}
	}

	sound_testing(); //TMP TODO REMOVE
}

