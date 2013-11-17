#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "space.h"
#include "../level.h"

STATE_ID state_editor;

static touchable *scr_world;
static touchable *scr_objects;

static button btn_space;
static button btn_clear;
static button btn_test;
static button btn_save;

static obj_param_tank tmp_tank_param = {
	500,
	50
};
static level *lvl;

static int remove_tool = 0;

static int active_obj_index = 0;
static char object_type[32];
static char param_name[32];
static char level_name[32];

typedef enum EDITOR_MODE {
	MODE_OBJECTS_ADD,
	MODE_OBJECTS_MOVE,
	MODE_OBJECTS_REMOVE,

	MODE_PARAMS_CREATE,
	MODE_PARAMS_EDIT,

	MODE_POLY_CREATE,
	MODE_POLY_MOVEPOINT,
	MODE_POLY_MOVEFULL,
	MODE_POLY_REMOVEFULL,
	MODE_POLY_REMOVEPOINT,

	MODE_SPRITE_ADD,
	MODE_SPRITE_REMOVE
	/*
	MODE_GAME_OBJECTS
		- add, move, (rotate), and remove objects
	MODE_OBJECT_PARAMS
		- create, set, copy, and paste object params, create specific sub-objects?
		- (change view_objects to view_params for list of available params of the selected game object, with an option to duplicate or create new params for that object_type)
	MODE_LAYERS
		- change current layer
		- be able to add, select, change, resize, rotate, remove layersystem's drawables (i.e. sprite_ext, filled polygon with/without outline) for current layer
	 */
} editor_mode;

static editor_mode current_mode = MODE_OBJECTS_ADD;


#define EDITOR_OBJECT_COUNT 5

//TODO show actual objects in list
static char object_names[EDITOR_OBJECT_COUNT][32] = {
		"TANK",
		"FACTORY",
		"TURRET",
		"ROCKET",
		"ROBOTARM"};

static char sprite_names[EDITOR_OBJECT_COUNT][32] = {
		"tankbody001",
		"factoryblue",
		"turretgun001",
		"rocket",
		"saw"};

static button btn_objects[EDITOR_OBJECT_COUNT];

static cpVect start;

static view *main_view;
extern obj_player * space_create_player(int id);
struct instance_dummy {
	instance ins;
	struct {

	} params;
};

static void update_instances(instance *obj, void *data)
{
	if(obj->TYPE != obj_id_player) {
		level_add_object_recipe_name(lvl, obj->TYPE->NAME, (char*)&(((struct instance_dummy *)obj)->params), obj->p_start,0);
	}
}

static void start_editor_level(void *unused)
{
	SDL_Log("EDITOR: STARTING LEVEL FROM EDITOR");
	llist_clear(lvl->level_data);
	instance_iterate(update_instances, NULL);
	statesystem_set_state(state_space);
	space_init_level_from_level(lvl);
}

static void save_level_to_file(void *unused)
{
	level_write_to_file(lvl);
}

static void select_object_type(void *index)
{
	Color col_active = {150,150,150,150};
	button_set_backcolor(btn_objects[active_obj_index],COL_WHITE);
	active_obj_index = *((int *)&index);
	button_set_backcolor(btn_objects[active_obj_index],col_active);
	strncpy(object_type, object_names[active_obj_index], 32);
}

static void clear_editor(void *unused)
{
	currentlvl = lvl;
	objectsystem_clear();
	space_create_player(1);
	select_object_type(0);
}


#define MAX_TOUCH_STACKSIZE 64

typedef struct finger_data {
	SDL_FingerID;
	int timestamp;
	int identifier;
	void *data;
} finger_data;

typedef struct editor_touch {
	cpVect first;
} editor_touch;

int touch_stack_i = 0;
editor_touch touch_stack[MAX_TOUCH_STACKSIZE];

static int touch_down(cpVect pos_view)
{
	//TODO map finger-id til et evt. objekt, ellers, returner null
	//TODO detect double tap for delete? eller bruke en state for sletting av objekter (ved trykk på knapp)
	//TODO create a system for registration of finger_id with an void* data, returning a unique id for touch (incrementing int)
	//FIXME vanskelig å zoome ut (evt. umulig) om man zoomer helt inn på et objekt (kan evt. fikses ved bruk av relativ flytting, og/eller minske største forstørring, bruk av timeout for touch, reset view knapp)
	start = pos_view;




	return 1;
}

static int touch_motion(cpVect pos_view)
{
	cpVect pos = view_view2world(main_view, pos_view);
	//TODO lage hjelpemetode for å hente objekt på posisjon
	cpNearestPointQueryInfo info;
	cpShape *shape = cpSpaceNearestPointQueryNearest(current_space, pos, 20, CP_ALL_LAYERS, CP_NO_GROUP, &info);
	if (shape) {
		instance *ins = shape->body->data;
		if (ins && ((ins->INS_IDENTIFIER ^ INS_MAGIC_COOKIE) == 0)) {
			ins->p_start = pos;
			ins->TYPE->call.init(ins);
			if(remove_tool) {
				instance_remove(ins);
			}
		}
		start = pos_view;
		return 1; /* consume event: no zoom/pan/rotate of scroller*/
	} else if (cpvlength(cpvsub(pos_view, start)) > 50) {
		return 0;
	}
}

static int touch_up(cpVect pos_view)
{
	cpVect pos = view_view2world(main_view, pos_view);
	//TODO lage hjelpemetode for å hente objekt på posisjon
	cpShape *shape = cpSpaceNearestPointQueryNearest(current_space, pos, 20, CP_ALL_LAYERS, CP_NO_GROUP, NULL);
	if (!shape) {
		//TODO make sure there are no other instances beneath pos!
		//level_add_object_recipe_name(lvl, object_type, param_name, pos,0);
		instance_create(object_by_name(object_type),
				level_get_param(lvl->param_list, object_type, param_name)
				,pos, cpvzero);
		return 1;
	}
	return 0;
}

void editor_init()
{
	sprintf(param_name, "%s", "DEF");
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
	for(i = 0; i<500; i++){
		int layer =  11 + roundf(we_randf*(layers-1-11));
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

		state_add_sprite(state_editor, layer, spr, size, size, pos, we_randf*WE_2PI);
	}

	state_enable_objects(state_editor, 1);

	cpSpaceSetGravity(current_space, cpv(0, 0));
	cpSpaceSetDamping(current_space, 0.8f);

	btn_space = button_create(SPRITE_BTN_HOME, 0, "", -400, GAME_HEIGHT/2 - 100, 125, 125);
	btn_clear = button_create(SPRITE_BTN_RETRY, 0, "", -200, GAME_HEIGHT/2-100, 125, 125);
	btn_test = button_create(SPRITE_BTN_NEXT, 0, "",0, GAME_HEIGHT/2 - 100, 125, 125);
	btn_save = button_create(SPRITE_COIN, 0, "",200, GAME_HEIGHT/2 -100, 125, 125);

	button_set_callback(btn_space, statesystem_set_state, state_stations);
	button_set_callback(btn_clear, clear_editor, 0);
	button_set_callback(btn_test, start_editor_level, 0); // TODO test level with this button
	button_set_callback(btn_save, save_level_to_file, 0);
	button_set_hotkeys(btn_test, KEY_RETURN_1, KEY_RETURN_2);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_clear, 1.5);
	button_set_enlargement(btn_test, 1.5);
	button_set_enlargement(btn_save, 1.5);

	state_register_touchable_view(main_view, btn_space);
	state_register_touchable_view(main_view, btn_clear);
	state_register_touchable_view(main_view, btn_test);
	state_register_touchable_view(main_view, btn_save);
	state_register_touchable_view(main_view, btn_settings);

	float size = 200;
	float x = -GAME_WIDTH/2+size*2;
	float y = GAME_HEIGHT/2-size*2;
	SDL_Scancode key = SDL_SCANCODE_1;
	for (i = 0; i < EDITOR_OBJECT_COUNT; i++, y -= 250, key++) {
		SPRITE_ID spr_id = sprite_link(sprite_names[i]);
		button btn = button_create(spr_id, 0, "", x, y, size, size);
		button_set_callback(btn, select_object_type, i);
		button_set_hotkeys(btn, key <= SDL_SCANCODE_0 ? key : 0, 0);
		button_set_enlargement(btn, 1.5);
		state_register_touchable_view(main_view, btn); //TODO use another view for these buttons
		btn_objects[i] = btn;
	}
	scr_world = scroll_create(150,0,GAME_WIDTH-300,GAME_HEIGHT, 0.98, 3000, 1, 1, 0); // max 4 000 gu / sec
	scr_objects = scroll_create(-GAME_WIDTH/2+150,0,300,GAME_HEIGHT,0.9,50,0,0,1);
	scroll_set_callback(scr_world, touch_down, touch_motion, touch_up);
	scroll_set_bounds(scr_objects, cpBBNew(0,-GAME_HEIGHT/2,0,GAME_HEIGHT/2));
	state_register_touchable_view(main_view, scr_objects);
	state_register_touchable_view(main_view, scr_world);

	lvl = level_load("object_defaults");
	clear_editor(NULL);
	state_enable_objects(state_editor, 0);
}

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
}

static void pre_update(void)
{
	main_view->zoom = scroll_get_zoom(scr_world);
	view_update(main_view, cpvneg(scroll_get_offset(scr_world)), scroll_get_rotation(scr_world));


	cpVect obj_offset = scroll_get_offset(scr_objects);
	//TODO use another view for objects_scroller
	int i;
	float x = -GAME_WIDTH/2+150;
	float y = GAME_HEIGHT/2-200 + obj_offset.y;
	for (i = 0; i < EDITOR_OBJECT_COUNT; i++, y -= 250) {
		touch_place(btn_objects[i], x, y);
	}
}

static void post_update(void)
{

}

void space_draw_deck(void);
static void draw(void)
{
	draw_color4f(1,1,1,1);
	tilemap_render(RLAY_BACK_BACK, currentlvl->tiles);
	space_draw_deck();
}

static int sdl_event(SDL_Event *event)
{
	SDL_TouchFingerEvent *finger = &event->tfinger;
	float zoom;
	SDL_Scancode key;
	cpVect pos;
	pos = view_touch2world(main_view, cpv(finger->x, finger->y));

	switch(event->type) {
	case SDL_KEYDOWN:
		key = event->key.keysym.scancode;
		if(key == SDL_SCANCODE_D) {
			remove_tool ^= 1;
		}
		break;
	}
	return 0;
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
