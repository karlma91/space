#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "space.h"
#include "../level.h"

STATE_ID state_editor;

static touchable *scroller;

static button btn_space;
static button btn_next;

static obj_param_tank tmp_tank_param = {
	500,
	50
};
static level *lvl;

static cpVect start;
static int dragged;

static view *main_view;

void editor_init()
{
	int i;
	statesystem_register(state_editor,0);
	main_view = state_view_get(state_editor, 0);
	state_add_layers(state_editor, 20);

	int layers = state_layer_count(state_editor);
	for(i = 11; i<layers; i++){
		//float depth =  2 + 10*tan((1.0f*i/la_sys->num_layers)*WE_PI_2);
		float f = (layers - i * 0.99f) / (layers);
		state_set_layer_parallax(state_editor, i, f, f);
	}
	for(i = 0; i<2000; i++){
		int layer =  11 + roundf(we_randf*(layers-1-11));
		float size = 150 + we_randf*90 - layer*4;
		cpVect pos = cpvmult(cpv(we_randf-0.5,we_randf-0.5),6600);
		SPRITE_ID spr;
		int s = rand() & 7;
		switch(s) {
		default: spr = SPRITE_SPIKEBALL; break;
		case 1: spr = SPRITE_COIN; break;
		case 2: spr = SPRITE_GEAR; break;
		case 3: spr = SPRITE_SAW; break;
		case 4: spr = SPRITE_PLAYERBODY001; break;
		case 5: spr = SPRITE_TANKWHEEL001; break;
		case 6: spr = SPRITE_TANKGUN001; break;
		case 7: spr = SPRITE_PLAYERGUN001; break;
		}
		state_add_sprite(state_editor, layer, spr, size, size, pos, we_randf*WE_2PI);
	}

	state_enable_objects(state_editor, 1);
	state_enable_particles(state_editor, 1);

	cpSpaceSetGravity(current_space, cpv(0, 0));
	cpSpaceSetDamping(current_space, 0.8f);

	scroller = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.98, 3000, 1, 0); // max 4 000 gu / sec

	btn_space = button_create(SPRITE_BTN_HOME, 0, "", -GAME_WIDTH/2 + 100, GAME_HEIGHT/2 - 100, 125, 125);
	btn_next = button_create(SPRITE_BTN_NEXT, 0, "",0, GAME_HEIGHT/2 - 100, 125, 125);

	button_set_callback(btn_space, statesystem_set_state, state_stations);
	//button_set_callback(btn_next, NULL, 0); // TODO test level with this button

	button_set_hotkeys(btn_next, KEY_RETURN_1, KEY_RETURN_2);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_next, 1.5);

	state_register_touchable_view(main_view, btn_space);
	state_register_touchable_view(main_view, btn_next);
	state_register_touchable_view(main_view, scroller);
	state_register_touchable_view(main_view, btn_settings);


	lvl = level_load("test");
	currentlvl = lvl;
}

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
	currentlvl = lvl;
	level_start_level(lvl);
}

static void pre_update(void)
{
	main_view->zoom = scroll_get_zoom(scroller);
	view_update(main_view, scroll_get_offset(scroller), scroll_get_rotation(scroller));
}

static void post_update(void)
{

}

static void draw(void)
{
}

static void sdl_event(SDL_Event *event)
{
	SDL_TouchFingerEvent *finger = &event->tfinger;
	float zoom;
	cpVect pos;

	switch(event->type) {
	case SDL_FINGERDOWN:
		start = main_view->p;
		dragged = 0;
		break;
	case SDL_FINGERMOTION:
		zoom = main_view->zoom;

		if(cpvlength(cpvsub(main_view->p, start)) > 50 / zoom){
			dragged = 1;
		}
		break;
	case SDL_FINGERUP:
		pos = cpv(finger->x, finger->y);
		if(!dragged) {
			instance_create(obj_id_tank, &tmp_tank_param,pos, cpvzero);
		}
		break;
	}
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

static void update(touchable *t)
{

}
static void render(touchable *t)
{

}
