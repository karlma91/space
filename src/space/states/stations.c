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

STATE_ID state_stations;

static button btn_home;
LList user_system;

static scroll_p scroller;

static Color col_unfocus = {40,40,65,0}; //{0,0,0,255};
static we_bool lost_focus = WE_FALSE;
static float focus_a = 0;

// TODO: cooler system
//tween test
static tween *test;
static tween *tests;
static tween *testr;
static cpVect a, b;
static float r;

static view *main_view;

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
    //TODO: TMP TWEEN TEST
    test = tween_cpv_create(cpv(0,300), cpv(500,500), 2, LinearInterpolation);
    tests = tween_cpv_create(cpv(50,50), cpv(100,100), 2, ElasticEaseInOut);
    tween_repeat(tests, 1, 1);
    float str = 0;
    float endr = 3.14;
    testr = tween_create(&(str), &(endr), 1, 3, ElasticEaseInOut);
    tween_repeat(testr, 2, 1);

    //TODO use tween for colors here!
    lost_focus = WE_FALSE;
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
	return 0;
}

static void draw(void)
{
	draw_color4f(1,1,1,1);
	//draw_box(1, a, b, r, 1);

	draw_push_matrix();
	draw_translate(1900,-1750);
	draw_rotate(-current_view->rotation);
	bmfont_center(FONT_SANS, cpvzero,1,"Space (working title)\nETA: 25. Jan 2014\n\nCredits:\nMathias Wilhelmsen\nKarl Magnus Kalvik\n\nAlpha Testers\nJacob & Jonathan Høgset [iPod 4th]\nBård-Kristian Krohg [iPod 3rd]");
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
    test = tween_release(test);
    tests = tween_release(tests);
    testr = tween_release(testr);

    lost_focus = WE_TRUE;
}

static void destroy(void)
{
}

static void open_upgrades(void *unused)
{
	statesystem_push_state(state_store);
}

void stations_init(void)
{
	srand(0x9b3a09fa);
	statesystem_register(state_stations, 0);

	user_system = solarsystem_load_solar("levels/userlevels.json");

	Color col_back = {30,100,30,255};

	main_view = state_view_get(state_stations, 0);
	main_view->GUI = draw_gui;

	btn_home = button_create(SPRITE_BTN1, 1, "$$$", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 200, 250, 250);
	button_set_click_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 0.9);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	button_set_backcolor(btn_home, col_back);
	state_register_touchable_view(main_view, btn_home);

	llist_begin_loop(user_system);
	while (llist_hasnext(user_system)) {
		solarsystem_register_touch((solarsystem *)llist_next(user_system), this);
	}
	llist_end_loop(user_system);

	state_register_touchable_view(main_view, btn_settings);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000, 1, 1, 0); // max 4 000 gu / sec
	scroll_set_bounds(scroller, cpBBNew(-60000, -60000, 60000, 60000));
	scroll_set_zoomlimit(scroller, 0.01, 1);
	scroll_set_zoom(scroller,0.5);
	state_register_touchable_view(main_view, scroller);

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

