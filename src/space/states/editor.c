#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "space.h"
#include "../level.h"
#include "../grid.h"

#define TAP_TIMEOUT 0.2
#define MIN_INNER_RADIUS 200
#define MIN_RADIUS_OFFSET 200
#define MAX_OUTER_RADIUS 10000

STATE_ID state_editor;

static touchable *scr_world;
static touchable *scr_objects;

static button btn_space;
static button btn_clear;
static button btn_test;
static button btn_save;
static button btn_delete;

static button btn_state_objects;
static button btn_state_resize;
static button btn_state_tilemap;

static polgrid *pgrid;

static level *lvl;

typedef struct editor_touch {
	touch_unique_id ID;
	cpVect viewpos_start, viewpos_last, game_offset;
	instance *ins;
    float time;
} editor_touch;

static int remove_tool = 0;

static int active_obj_index = 0;
static char object_type[32];
static char level_name[32];
static char *param_name;

static LList ll_touches;
static pool *pool_touches;

typedef enum EDITOR_MODE {

	MODE_OBJECTS,
	MODE_TILEMAP,
	MODE_RESIZE
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

touch_unique_id resize_finger;
cpVect resize_prev;
enum RESIZE_MODE {
	RESIZE_NONE,
	RESIZE_INNER,
	RESIZE_OUTER
};

static editor_mode current_mode = MODE_RESIZE;

static enum RESIZE_MODE resize_mode = RESIZE_NONE;


#define EDITOR_OBJECT_COUNT 8

//TODO show actual objects in list
static char object_names[EDITOR_OBJECT_COUNT][32] = {
		"CRATE",
		"TANK",
		"FACTORY",
		"FACTORY",
		"FACTORY",
		"TURRET",
		"ROCKET",
		"ROBOTARM"};
static char param_names[EDITOR_OBJECT_COUNT][32] = {
		"def",
		"DEF",
		"DEF",
		"DEF_RED",
		"DEF_RAMP",
		"DEF",
		"DEF",
		"DEF"};

static char sprite_names[EDITOR_OBJECT_COUNT][32] = {
		"gear",
		"tankbody001",
		"factoryblue",
		"factoryred",
		"ramp",
		"turretgun001",
		"rocket",
		"saw"};

static button btn_objects[EDITOR_OBJECT_COUNT];

static view *view_editor;
extern obj_player * space_create_player(int id);
struct instance_dummy {
	instance ins;
	struct {

	} params;
};

static void update_instances(instance *obj, void *data)
{
		level_add_object_recipe_name(lvl, obj->TYPE->NAME, (char*)&(((struct instance_dummy *)obj)->params), obj->p_start,0);
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
	llist_clear(lvl->level_data);
	instance_iterate(update_instances, NULL);
	level_write_to_file(lvl);
}

static void select_object_type(void *index)
{
	Color col_active = {150,150,150,150};
	button_set_backcolor(btn_objects[active_obj_index],COL_WHITE);
	active_obj_index = *((int *)&index);
	button_set_backcolor(btn_objects[active_obj_index],col_active);
	strncpy(object_type, object_names[active_obj_index], 32);
	param_name = param_names[active_obj_index];
}

static void enable_objlist(int enable)
{
	if (enable) {

	} else {

	}
}

static void tap_clear_editor(void *unused)
{
	currentlvl = lvl;
	objectsystem_clear();
	space_create_player(1);
	select_object_type(0);

	cpVect p = cpvzero;
	instance *player = instance_first(obj_id_player);
	if (player) p = player->body->p;
	view_editor->zoom = 1;
	view_update(view_editor, p, 0);
	remove_tool = 0;
	enable_objlist(!remove_tool);
}

static void btn_set_editor_mode(editor_mode state)
{
	current_mode = state;
}

static void tap_delete_click(void *unused)
{
	remove_tool = !remove_tool;
	enable_objlist(!remove_tool);
}

static int contains_instance(instance *ins)
{
	llist_begin_loop(ll_touches);
	while (llist_hasnext(ll_touches)) {
		editor_touch *touch = (editor_touch *) llist_next(ll_touches);
		if (touch->ins == ins) {
			llist_end_loop(ll_touches);
			return 1;
		}
	}
	llist_end_loop(ll_touches);
	return 0;
}

static editor_touch *get_touch(SDL_FingerID finger_id)
{
	touch_unique_id id = finger_get_touch_id(finger_id);
	llist_begin_loop(ll_touches);
	while (llist_hasnext(ll_touches)) {
		editor_touch *touch = (editor_touch *) llist_next(ll_touches);
		if (touch->ID == id) {
			llist_end_loop(ll_touches);
			return touch;
		}
	}
	llist_end_loop(ll_touches);
	return NULL;
}

static void move_instance(editor_touch *touch)
{
	cpVect pos = view_view2world(view_editor, touch->viewpos_last);
	touch->ins->p_start = cpvadd(pos, touch->game_offset);
	touch->ins->TYPE->call.init(touch->ins);
	if (remove_tool) {
		instance_remove(touch->ins);
		finger_unbind(touch->ID);
		llist_remove(ll_touches, touch);
		pool_release(pool_touches, touch);
	}
}

static int touch_down(SDL_TouchFingerEvent *finger)
{
	//TODO detect double tap for delete? eller bruke en state for sletting av objekter (ved trykk på knapp)

	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	instance *ins;
	float l;

	switch (current_mode) {
	case MODE_OBJECTS:
		ins = instance_at_pos(pos, 10/view_editor->zoom, CP_ALL_LAYERS, CP_NO_GROUP); //TODO use zoom to decide max distance?
		if (contains_instance(ins)) {
			return 0;
		}
		touch_unique_id touch_id = finger_bind(finger->fingerId);
		if (touch_id != -1) {
			editor_touch *touch = pool_instance(pool_touches);
			llist_add(ll_touches, touch);
			touch->viewpos_start = pos_view;
			touch->viewpos_last = pos_view;
			touch->ID = touch_id;
			touch->ins = ins;
			touch->time = 0;
			if (ins) {
				//TODO don't allow more than one(or two for rotate/scale of instance) binding per instance!
				touch->game_offset = cpvsub(ins->body->p, pos);
				move_instance(touch);
			}
			return 1; /* consume event: no zoom/pan/rotate of scroller*/
		}
		break;
	case MODE_RESIZE:
		l = cpvlength(pos);
		if ( l < currentlvl->inner_radius + 10/view_editor->zoom){
			resize_mode = RESIZE_INNER;
			resize_prev = pos;
			resize_finger = finger_bind(finger->fingerId);
		}else if(l > currentlvl->outer_radius + 10/view_editor->zoom){
			resize_mode = RESIZE_OUTER;
			resize_prev = pos;
			resize_finger = finger_bind(finger->fingerId);
		}

		break;
	case MODE_TILEMAP:

		break;
	}
	return 0;
}

static int touch_motion(SDL_TouchFingerEvent *finger)
{
	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	editor_touch *touch;
	float new_r;
	switch(current_mode) {
	case MODE_OBJECTS:
		touch = get_touch(finger->fingerId);
		if (touch) {
			instance *ins = touch->ins;
			if (ins) {
				if(remove_tool) { //TODO create use inner state to decide what to do
					instance_remove(touch->ins);
				} else {
					touch->viewpos_last = pos_view;
					move_instance(touch);
				}
			} else if (cpvdistsq(touch->viewpos_start, pos_view) > 20*20) {
				finger_unbind(touch->ID);
				llist_remove(ll_touches, touch);
				pool_release(pool_touches, touch);
			}
			return 1;
		}
		return 0;
	case MODE_RESIZE:
		if(resize_finger == finger_get_touch_id(finger->fingerId)){
			if (resize_mode == RESIZE_INNER) {
				new_r =  currentlvl->inner_radius + (cpvlength(pos) - cpvlength(resize_prev));
				if(new_r >= MIN_INNER_RADIUS && new_r <= currentlvl->outer_radius - MIN_RADIUS_OFFSET) {
					currentlvl->inner_radius = new_r;
					grid_update(pgrid, pgrid->cols, currentlvl->inner_radius, currentlvl->outer_radius);
				}
				resize_prev=pos;
			} else if(resize_mode == RESIZE_OUTER) {
				new_r =  currentlvl->outer_radius + (cpvlength(pos) - cpvlength(resize_prev));
				if(new_r <= MAX_OUTER_RADIUS && new_r >= currentlvl->inner_radius + MIN_RADIUS_OFFSET) {
					currentlvl->outer_radius = new_r;
					grid_update(pgrid, pgrid->cols, currentlvl->inner_radius, currentlvl->outer_radius);
				}
				resize_prev=pos;
			}
		}else{
			resize_mode = RESIZE_NONE;
		}
		break;
	case MODE_TILEMAP:
		break;
	}
	return 0;
}

static int touch_up(SDL_TouchFingerEvent *finger)
{
	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	editor_touch *touch = get_touch(finger->fingerId);
	if (touch) {
		if (touch->ins == NULL) {
			//TODO make sure there are no other instances beneath pos!
			instance_create(object_by_name(object_type), level_get_param(&(lvl->params), object_type, param_name), pos, cpvzero);
			llist_remove(ll_touches, touch);
			pool_release(pool_touches, touch);
			return 1;
		}
	}
	return 0;
}

static void draw_gui(view *v)
{
	bmfont_center(FONT_COURIER, cpv(0,-WINDOW_HEIGHT/2),1,"MODE: %d",(int)current_mode);
}

void editor_init()
{
	sprintf(object_type, "%s", "TURRET");
	sprintf(level_name, "%s", "TESTING");
	int i;
	statesystem_register(state_editor,0);
	view_editor = state_view_get(state_editor, 0);
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
		byte l = 255;//255 - 200 * i / layers;
		Color col = {l,l,l,0};
		cpVect pos = cpvmult(cpv(we_randf-0.5,we_randf-0.5),6600);
		SPRITE_ID spr;
		int s = rand() & 7;
		switch(s) {
        default: spr = SPRITE_DOT; break;
		//case 1: spr = SPRITE_GEAR; break;
		//case 2: spr = SPRITE_STATION001; break;
		//case 3: spr = SPRITE_TANKWHEEL001; break;
		}

		state_add_sprite(state_editor, layer, spr, size, size, pos, we_randf*WE_2PI,col);
	}

	state_enable_objects(state_editor, 1);
	state_enable_objupdate(state_editor, 0);
	state_enable_physics(state_editor, 1);

	view_editor->GUI = draw_gui;

	cpSpaceSetGravity(current_space, cpv(0, 0));
	cpSpaceSetDamping(current_space, 0);

	ll_touches = llist_create();
	pool_touches = pool_create(sizeof(editor_touch));

	btn_space  = button_create(SPRITE_BTN_HOME, 0, "", -400, GAME_HEIGHT/2 - 100, 125, 125);
	btn_clear  = button_create(SPRITE_BTN_RETRY, 0, "", -200, GAME_HEIGHT/2-100, 125, 125);
	btn_test   = button_create(SPRITE_BTN_NEXT, 0, "",0, GAME_HEIGHT/2 - 100, 125, 125);
	btn_save   = button_create(SPRITE_COIN, 0, "",200, GAME_HEIGHT/2 -100, 125, 125);
	btn_delete = button_create(SPRITE_SPIKEBALL, 0, "X", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 100, 125, 125);

	btn_state_objects = button_create(SPRITE_SPIKEBALL, 0, "Objects", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 400, 125, 125);
	btn_state_resize = button_create(SPRITE_SPIKEBALL, 0, "Resize", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 300, 125, 125);
	btn_state_tilemap = button_create(SPRITE_SPIKEBALL, 0, "Tilemap", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 200, 125, 125);

	button_set_callback(btn_state_objects, btn_set_editor_mode, MODE_OBJECTS);
	button_set_callback(btn_state_resize, btn_set_editor_mode,  MODE_RESIZE);
	button_set_callback(btn_state_tilemap, btn_set_editor_mode,  MODE_TILEMAP);

	button_set_callback(btn_space, statesystem_set_state, state_stations);
	button_set_callback(btn_clear, tap_clear_editor, 0);
	button_set_callback(btn_test, start_editor_level, 0); // TODO test level with this button
	button_set_callback(btn_save, save_level_to_file, 0);
	button_set_callback(btn_save, save_level_to_file, 0);
	button_set_callback(btn_delete, tap_delete_click, 0);

	button_set_hotkeys(btn_test, KEY_RETURN_1, KEY_RETURN_2);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);
	button_set_hotkeys(btn_delete, SDLK_DELETE, SDLK_BACKSPACE);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_clear, 1.5);
	button_set_enlargement(btn_test, 1.5);
	button_set_enlargement(btn_save, 1.5);
	button_set_enlargement(btn_delete, 1.5);
	button_set_enlargement(btn_state_objects, 1.2);
	button_set_enlargement(btn_state_resize, 1.2);
	button_set_enlargement(btn_state_tilemap, 1.2);
	button_set_backcolor(btn_delete, COL_RED);

	state_register_touchable_view(view_editor, btn_space);
	state_register_touchable_view(view_editor, btn_clear);
	state_register_touchable_view(view_editor, btn_test);
	state_register_touchable_view(view_editor, btn_save);
	state_register_touchable_view(view_editor, btn_settings);
	state_register_touchable_view(view_editor, btn_delete);
	state_register_touchable_view(view_editor, btn_state_objects);
	state_register_touchable_view(view_editor, btn_state_resize);
	state_register_touchable_view(view_editor, btn_state_tilemap);

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
		state_register_touchable_view(view_editor, btn); //TODO use another view for these buttons
		btn_objects[i] = btn;
	}
	scr_world = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.9, 3000, 1, 1, 0); // max 4 000 gu / sec
	scr_objects = scroll_create(-GAME_WIDTH/2+150,0,300,GAME_HEIGHT,0.9,50,0,0,1);
	scroll_set_callback(scr_world, touch_down, touch_motion, touch_up);
	scroll_set_bounds(scr_objects, cpBBNew(0,-GAME_HEIGHT/2,0,EDITOR_OBJECT_COUNT * 200));
	state_register_touchable_view(view_editor, scr_objects);
	state_register_touchable_view(view_editor, scr_world);

	lvl = level_load("empty");
	tap_clear_editor(NULL);

	pgrid = grid_create(32, currentlvl->inner_radius, currentlvl->outer_radius);
}

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
}

static void pre_update(void)
{
	view_editor->zoom = scroll_get_zoom(scr_world);
	view_update(view_editor, cpvneg(scroll_get_offset(scr_world)), scroll_get_rotation(scr_world));

	float panel_offset = (remove_tool || current_mode == MODE_RESIZE || current_mode == MODE_TILEMAP ? -400 : 0);
	touch_place(scr_objects,-GAME_WIDTH/2+150+panel_offset,0);
	cpVect obj_offset = scroll_get_offset(scr_objects);
	//TODO use another view for objects_scroller
	int i;
	float x = -GAME_WIDTH/2+150 + panel_offset;
	float y = GAME_HEIGHT/2-200 + obj_offset.y;
	for (i = 0; i < EDITOR_OBJECT_COUNT; i++, y -= 250) {
		touch_place(btn_objects[i], x, y);
	}

	llist_begin_loop(ll_touches);
	while (llist_hasnext(ll_touches)) {
		editor_touch *touch = (editor_touch *) llist_next(ll_touches);
		if (finger_status(touch->ID, -1)) {
            touch->time += dt;
            if (touch->ins) {
                move_instance(touch);
            } else if (touch->time >= TAP_TIMEOUT) {
                finger_unbind(touch->ID);
            }
		} else {
			llist_remove(ll_touches, touch);
			pool_release(pool_touches, touch);
		}
	}
	llist_end_loop(ll_touches);
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

	draw_color4f(1,1,1,0.4);
	if ((current_mode == MODE_RESIZE) || (current_mode == MODE_TILEMAP)) {
		grid_draw(pgrid, 0, 20);
	}
}

static int sdl_event(SDL_Event *event)
{
	SDL_TouchFingerEvent *finger = &event->tfinger;
	//SDL_Scancode key;
	cpVect pos;
	pos = view_touch2world(view_editor, cpv(finger->x, finger->y));

	switch(event->type) {
	case SDL_KEYDOWN:
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
	objectsystem_clear();
	llist_destroy(ll_touches);
	pool_destroy(pool_touches);
}
