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

static SPRITE_ID spr_sun;

static button *btn_stations;
static button btn_home;
static button btn_editor;

static level_ship *stations;
static int station_count = 2;

#define SOLSYS_COUNT 25
static solarsystem *solsys[SOLSYS_COUNT];

static scroll_p scroller;

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
	//draw_box(1, a, b, r, 1);

	draw_push_matrix();
	draw_translate(1900,-1750);
	draw_rotate(-current_view->rotation);
	bmfont_center(FONT_COURIER, cpvzero,1,"Space (working title)\nETA: 25. Jan 2014\n\nCredits:\nMathias Wilhelmsen\nKarl Magnus Kalvik\n\nAlpha Testers\nJacob & Jonathan Høgset [iPod 4th]\nBård-Kristian Krohg [iPod 3rd]");
	draw_pop_matrix();

	int i;
	for (i=0; i<SOLSYS_COUNT; i++) {
		solarsystem_draw(solsys[i]);
	}

	/*
	float f = 1/2.0;
	float size_dither = (engine_time*f - floor(engine_time*f) - 0.5)*(engine_time*f - floor(engine_time*f) - 0.5)*10;
	cpVect sun_size = {400+size_dither,400+size_dither};
	static float a_sun_add1 = 0;
	static float a_sun_add2 = 0;
    static float spd = 0.03;
	a_sun_add1 += dt*WE_2PI*spd;
	a_sun_add2 -= dt*WE_2PI*spd;

	//draw_color4b(100,100,100,0);
	//sprite_render_index_by_id(RLAY_GAME_BACK, SPRITE_GLOW, 0, cpvzero, cpvmult(sun_size,1.5), a_sun_base);
    static Color sun_base = {0x70,0x30,0x30,0xff};
    static Color sun_glow = {0xff,0xa0,0x70,0x80};
    static Color sun_add1 = {0x90,0x80,0x40,0x00};
    static Color sun_add2 = {0xb0,0x70,0x40,0x00};

    static Color sun2_glow = {0xe0,0xa0,0x70,0x80};
    static Color sun2_add1 = {0x40,0x80,0x90,0x00};
    static Color sun2_add2 = {0x40,0x70,0xb0,0x00};
    draw_push_matrix();
    draw_translate(6000, 2000);
    draw_scale(5, 5);
	draw_color(sun_base);
	sprite_render_index_by_id(RLAY_GAME_BACK, spr_sun, 0, cpvzero, sun_size, 0);
	draw_color(sun2_glow);
	sprite_render_index_by_id(RLAY_GAME_BACK, SPRITE_GLOW, 0, cpvzero, cpvmult(sun_size,2), 0);
	draw_color(sun2_add1);
	sprite_render_index_by_id(RLAY_GAME_BACK, spr_sun, 1, cpvzero, sun_size, a_sun_add1);
	draw_color(sun2_add2);
	sprite_render_index_by_id(RLAY_GAME_BACK, spr_sun, 2, cpvzero, sun_size, a_sun_add2);
    draw_pop_matrix();
    */
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
	srand(0x9b3a09fa);
	statesystem_register(state_stations, 0);

	level_get_ships(&stations, &station_count);

	btn_stations = calloc(station_count, sizeof(button));

	Color col_back = {255,180,140,255};
	//Color col_text = {1,1,1,1};

	main_view = state_view_get(state_stations, 0);

	btn_home = button_create(NULL, 0, "Upgrades", -GAME_WIDTH/4, -GAME_HEIGHT/2 + 80, 200, 200);
	button_set_callback(btn_home, open_upgrades, 0);
	button_set_enlargement(btn_home, 2);
	button_set_hotkeys(btn_home, KEY_RETURN_1, KEY_RETURN_2);
	state_register_touchable_view(main_view, btn_home);

	btn_editor = button_create(NULL, 0, "Editor", GAME_WIDTH/4, -GAME_HEIGHT/2 + 80, 873*2/5, 247*2/5);
	button_set_callback(btn_editor, open_editor, 0);
	button_set_enlargement(btn_editor, 2);
	button_set_hotkeys(btn_editor, SDL_SCANCODE_E, -1);
	state_register_touchable_view(main_view, btn_editor);

	int i;
	for (i = 0; i < station_count; i++) {
		char stri[10];
		sprintf(stri, "%d", i+1);
		float size = 300 + we_randf * 100;
		cpFloat radius = (i+2) * 450;
		cpFloat angle = WE_2PI * we_randf;
		stations[i].pos = WE_P2C(radius,angle);

		btn_stations[i] = button_create(SPRITE_STATION001, 0, stations[i].level_name, stations[i].pos.x,stations[i].pos.y, size, size);
		button_set_callback(btn_stations[i], button_callback, &stations[i]);
		button_set_txt_antirot(btn_stations[i], 1);
		button_set_backcolor(btn_stations[i], col_back);
		button_set_animated(btn_stations[i], 1, (i ? 18 : 15));
		button_set_enlargement(btn_stations[i], 1.5);
		button_set_hotkeys(btn_stations[i], digit2scancode[(i+1) % 10], 0);
		sprite *spr = button_get_sprite(btn_stations[i]);
		spr->antirot = 1;

		state_register_touchable(this, btn_stations[i]);
	}

	state_register_touchable_view(main_view, btn_settings);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000, 1, 1, 0); // max 4 000 gu / sec
	scroll_set_bounds(scroller, cpBBNew(-60000, -60000, 60000, 60000));
	scroll_set_zoomlimit(scroller, 0.01, 1);
	scroll_set_zoom(scroller,0.5);
	state_register_touchable_view(main_view, scroller);

	state_add_layers(state_stations, 22);

	int layers = state_layer_count(state_stations);

	for(i = 11; i<layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
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

	//TMP add solar systems
	spr_sun = sprite_link("sun01");
	for (i=0; i<SOLSYS_COUNT; i++) {
		float rnd = rand() & 0x1f;
		Color base = {0x70-rnd,0x30,0x30+rnd,0xff};
		Color glow = {0xff-rnd,0xa0,0x70+rnd,0x80};
		Color add1 = {0x90-rnd,0x80,0x40+rnd,0x00};
		Color add2 = {0xb0-rnd,0x70,0x40+rnd,0x00};
		solsys[i] = solarsystem_create(main_view, i, (500 + we_randf*(i*1500/SOLSYS_COUNT + (i>1?1000:0)) + 300*i/SOLSYS_COUNT), spr_sun, base, glow, add1, add2);
	}

	sound_testing();
}

