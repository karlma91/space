#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "space.h"
#include "../level.h"

STATE_ID state_editor;

static touchable *scroller;

static button btn_space;
static button btn_next;
static button btn_save;

static obj_param_tank tmp_tank_param = {
	500,
	50
};
static level *lvl;

static char object_type[32];
static char object_name[32];
static char level_name[32];

static cpVect start;
static int dragged;

static view *main_view;

static void start_editor_level(void *data) {
	statesystem_set_state(state_space);
	space_init_level_from_level(lvl);
}

static void save_level_to_file(void *data) {
	level_write_to_file(lvl);
}


void editor_init()
{
	sprintf(object_name, "%s", "DEF");
	sprintf(object_type, "%s", "TURRET");
	sprintf(level_name, "%s", "TESTING");
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
	button_set_callback(btn_space, statesystem_set_state, state_stations);

	btn_next = button_create(SPRITE_BTN_NEXT, 0, "",0, GAME_HEIGHT/2 - 100, 125, 125);
	button_set_callback(btn_next, start_editor_level, 0); // TODO test level with this button

	btn_save = button_create(SPRITE_COIN, 0, "",300, GAME_HEIGHT/2 -100, 125, 125);
	button_set_callback(btn_save, save_level_to_file, 0); // TODO test level with this button

	button_set_hotkeys(btn_next, KEY_RETURN_1, KEY_RETURN_2);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_next, 1.5);
	button_set_enlargement(btn_save, 1.5);

	state_register_touchable_view(main_view, btn_space);
	state_register_touchable_view(main_view, btn_next);
	state_register_touchable_view(main_view, btn_save);
	state_register_touchable_view(main_view, scroller);
	state_register_touchable_view(main_view, btn_settings);


	lvl = level_load("object_defaults");
	currentlvl = lvl;

	state_enable_objects(state_editor, 0);
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
	SDL_Scancode key;
	cpVect pos;

	switch(event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;
		break;
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
		pos = camera_vect_view2world(main_view, cpv(finger->x, finger->y));

		if(!dragged) {
			level_add_object_recipe_name(lvl, object_type, object_name, pos,0);
			instance_create(object_by_name(object_type),
					level_get_param(lvl->param_list, object_type, object_name)
					,pos, cpvzero);
		}
		break;
	}


	switch(key) {
	case SDL_SCANCODE_1: sprintf(object_type, "%s", "TURRET"); break;
	case SDL_SCANCODE_2: sprintf(object_type, "%s", "TANK"); break;
	case SDL_SCANCODE_3: sprintf(object_type, "%s", "FACTORY"); break;
	case SDL_SCANCODE_4: sprintf(object_type, "%s", "ROCKET"); break;
	case SDL_SCANCODE_5: sprintf(object_type, "%s", "ROBOTARM"); break;
	default: break;

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
