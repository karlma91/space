#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "we_data.h"
#include "space.h"
#include "../level.h"


#define DRAG_LIMIT 50
#define TAP_TIMEOUT 0.2
#define MIN_INNER_RADIUS 200
#define MIN_RADIUS_OFFSET 200
#define MAX_OUTER_RADIUS 10000

#define OBJECT_MARGIN 10
#define WALL_MARGIN 30

STATE_ID state_editor;


/*********** GRID AND TILEDATA ***********/
static tile_layers current_tlay = TLAY_SOLID;
static grid_index grid_i_cur = {-1,-1};

/*********** LEVEL DATA ***********/
static level *lvl;
static char level_name[32];


/*********** EDITOR STATE ***********/
typedef enum EDITOR_MODE {
	MODE_RESIZE,
	MODE_TILEMAP,
	MODE_OBJECTS
} editor_mode;
static editor_mode current_mode = MODE_RESIZE;


/*********** RESIZE_MODE ***********/
typedef enum RESIZE_MODE {
	RESIZE_NONE,
	RESIZE_INNER,
	RESIZE_OUTER
} RESIZE_MODE;


/*********** TILE_MODE ***********/
typedef enum TILE_MODE {
	TILE_NONE,
	TILE_ADD,
	TILE_CLEAR
} TILE_MODE;


/*********** OBJECT MODE ***********/
static void select_object_type(void *index);
#define PANEL_WIDTH 300
#define EDITOR_OBJECT_COUNT 8
static int remove_tool = 0;
static int active_obj_index = 0;
static char object_type[32], *param_name;
static float btn_xoffset[EDITOR_OBJECT_COUNT];
static float panel_offset = -400;

//TODO render objects as they are actually rendered in object list
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

struct instance_dummy {
	instance ins;
	struct {} params;
};


/*********** GUI ELEMENTS AND INTERACTION ***********/
static view *view_editor;
static touchable *scr_world, *scr_objects;
static button btn_space, btn_clear, btn_test, btn_save;
static button btn_state_resize;
static button btn_state_tilemap, btn_layer;
static button btn_state_objects, btn_delete, btn_objects[EDITOR_OBJECT_COUNT];
static LList ll_touches;
static pool *pool_touches;

typedef union MODE_DATA {
	instance *object_ins;
	TILE_MODE tile_mode;
	RESIZE_MODE resize_mode;
	void *mode_data;
} MODE_DATA;
typedef struct editor_touch {
	editor_mode mode;
	touch_unique_id ID;
	cpVect viewpos_start, viewpos_cur, game_offset, viewpos_prev;
	MODE_DATA data;
	we_bool data_obj_delete;
    float time;
} editor_touch;


static editor_touch *add_touchdata(editor_mode m, touch_unique_id ID, cpVect view_start, cpVect offset, MODE_DATA data)
{
	editor_touch *touch = pool_instance(pool_touches);
	llist_add(ll_touches, touch);
	touch->mode = m;
	touch->viewpos_start = view_start;
	touch->viewpos_cur = view_start;
	touch->viewpos_prev = view_start;
	touch->ID = ID;
	touch->data = data;
	touch->time = 0;
	return touch;
}

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
	tilemap_fill(NULL, TLAY_COUNT, NULL, &(lvl->tilemap));
	level_write_to_file(lvl);
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
	extern obj_player * space_create_player(int id);
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

static void editor_setmode(editor_mode state)
{
	current_mode = state;

	we_bool visible;

	/* MODE_OBJECTS */
	visible = (current_mode == MODE_OBJECTS);
	btn_delete->visible = visible;
	remove_tool = 0;

	/* MODE_RESIZE */
	visible = (current_mode == MODE_RESIZE);
	//btn_layer->visible = visible;

	/* MODE_TILEMAP */
	visible = (current_mode == MODE_TILEMAP);
	btn_layer->visible = visible;
}

static void setlayer(button btn)
{
	current_tlay++;
	current_tlay = current_tlay >= TLAY_COUNT ? 0 : current_tlay;
	char number[2] = {('0'+(char)current_tlay), '\0'};
	button_set_text(btn, number);
}



static void tap_delete_click(void *unused)
{
	remove_tool = !remove_tool;
	enable_objlist(!remove_tool);
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

static RESIZE_MODE radius_at_pos(cpVect pos, float margin)
{
	float l = cpvlength(pos);
	if (lvl->inner_radius - margin < l && l < lvl->inner_radius + margin ) {
		fprintf(stderr, "DEBUG: RESIZE_INNER\n");
		return RESIZE_INNER;
	} else if (lvl->outer_radius - margin < l && l < lvl->outer_radius + margin ) {
		fprintf(stderr, "DEBUG: RESIZE_OUTER\n");
		return RESIZE_OUTER;
	}
	fprintf(stderr, "DEBUG: NONE_RESIZE\n");
	return RESIZE_NONE;
}

static void move_radius(editor_touch *touch)
{
	cpVect pos = view_view2world(view_editor, touch->viewpos_cur);
	cpVect prev = view_view2world(view_editor, touch->viewpos_prev);
	if (touch->data.resize_mode == RESIZE_INNER) {
		float new_r = lvl->inner_radius + (cpvlength(pos) - cpvlength(prev));
		if(new_r >= MIN_INNER_RADIUS && new_r <= lvl->outer_radius - MIN_RADIUS_OFFSET) {
			lvl->inner_radius = new_r;
			grid_setregion2f(lvl->tilemap.grid, lvl->inner_radius, lvl->outer_radius);
		}
	} else if(touch->data.resize_mode == RESIZE_OUTER) {
		float new_r = lvl->outer_radius + (cpvlength(pos) - cpvlength(prev));
		if(new_r <= MAX_OUTER_RADIUS && new_r >= lvl->inner_radius + MIN_RADIUS_OFFSET) {
			lvl->outer_radius = new_r;
			grid_setregion2f(lvl->tilemap.grid, lvl->inner_radius, lvl->outer_radius);
		}
	}
}

static void paint_tile(editor_touch *touch)
{
	cpVect pos = view_view2world(view_editor, touch->viewpos_cur);
	grid_index grid_i = grid_getindex(lvl->tilemap.grid, pos);
	grid_i_cur = grid_i;
	if (grid_i.yrow != -1) {
		if (touch->data.tile_mode == TILE_ADD) {
			tilemap_settile(&lvl->tilemap, current_tlay, grid_i.xcol, grid_i.yrow, 1);

			if (current_tlay == TLAY_SOLID && !lvl->tilemap.blocks[grid_i.yrow][grid_i.xcol]) {
				cpVect verts[4];
				grid_getquad8cpv(lvl->tilemap.grid, verts, grid_i.xcol, grid_i.yrow);
				cpShape *shape = cpPolyShapeNew(current_space->staticBody, 4, verts, cpvzero);
				lvl->tilemap.blocks[grid_i.yrow][grid_i.xcol] = shape;
				cpSpaceAddStaticShape(current_space, shape);
			}
		} else if (touch->data.tile_mode == TILE_CLEAR) {
			tilemap_settile(&lvl->tilemap, current_tlay, grid_i.xcol, grid_i.yrow, 0);

			if (current_tlay == TLAY_SOLID && lvl->tilemap.blocks[grid_i.yrow][grid_i.xcol]) {
				cpSpaceRemoveShape(current_space, lvl->tilemap.blocks[grid_i.yrow][grid_i.xcol]);
				cpfree(lvl->tilemap.blocks[grid_i.yrow][grid_i.xcol]);
				lvl->tilemap.blocks[grid_i.yrow][grid_i.xcol] = NULL;
			}
		}
	}
}

static editor_touch *contains_tile_finger(void)
{
	llist_begin_loop(ll_touches);
	while (llist_hasnext(ll_touches)) {
		editor_touch *touch = (editor_touch *) llist_next(ll_touches);
		if (touch->mode == MODE_TILEMAP) {
			llist_end_loop(ll_touches);
			return touch;
		}
	}
	llist_end_loop(ll_touches);
	return NULL;
}

static int contains_instance(instance *ins)
{
	llist_begin_loop(ll_touches);
	while (llist_hasnext(ll_touches)) {
		editor_touch *touch = (editor_touch *) llist_next(ll_touches);
		if (touch->mode == MODE_OBJECTS && touch->data.object_ins == ins) {
			llist_end_loop(ll_touches);
			return 1;
		}
	}
	llist_end_loop(ll_touches);
	return 0;
}


static we_bool is_deletable(instance *ins)
{
	return ins && ins->TYPE != obj_id_player;
}

static void delete_instance(editor_touch *touch)
{
	if (is_deletable(touch->data.object_ins)) {
		instance_remove(touch->data.object_ins);
		fprintf(stderr, "DEBUG: deleting instance of type: %s\n", touch->data.object_ins->TYPE->NAME);
	}
	touch->data.object_ins = NULL;
	touch->data_obj_delete = WE_FALSE;
}

static void move_instance(editor_touch *touch)
{
	cpVect pos = view_view2world(view_editor, touch->viewpos_cur);
	if (touch->data.object_ins) {
		if (!remove_tool) {
			// check if pos is inside station walls
			grid_index grid_i = grid_getindex(lvl->tilemap.grid,pos);
			if ((grid_i.yrow == -1) || (touch->viewpos_cur.x < (panel_offset + PANEL_WIDTH - view_editor->view_width/2))) {
				if (is_deletable(touch->data.object_ins)) {
					/* Jiggles instance to indicate that it will get removed on touch_up */
					pos = cpvadd(pos, cpvmult(cpv(we_randf-0.5,we_randf-0.5), 20));
					touch->data_obj_delete = WE_TRUE;
				} else {
					/* making sure non-deletable instances are not moved outside of station */
					float new_rad = grid_inner_radius(lvl->tilemap.grid);
					new_rad = (grid_i.dist_sq <= new_rad*new_rad) ? new_rad : grid_outer_radius(lvl->tilemap.grid);
					pos = WE_P2C(new_rad, grid_i.angle);
				}
			} else {
				touch->data_obj_delete = WE_FALSE;
			}
			touch->data.object_ins->p_start = cpvadd(pos, touch->game_offset);
			touch->data.object_ins->TYPE->call.init(touch->data.object_ins);
		} else {
			delete_instance(touch);
		}
	} else {
		touch->data.object_ins = instance_at_pos(pos, OBJECT_MARGIN/view_editor->zoom, CP_ALL_LAYERS, CP_NO_GROUP);
	}
}

static void select_object_type(void *index)
{
	//Color col_active = {100,100,100,100};
	//button_set_backcolor(btn_objects[active_obj_index],col_active);
	active_obj_index = *((int *)&index);
	//button_set_backcolor(btn_objects[active_obj_index],COL_WHITE);
	strncpy(object_type, object_names[active_obj_index], 32);
	param_name = param_names[active_obj_index];
}

static int touch_down(SDL_TouchFingerEvent *finger)
{
	//TODO detect double tap for delete? eller bruke en state for sletting av objekter (ved trykk på knapp)

	MODE_DATA data;
	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	grid_index grid_i = grid_getindex(lvl->tilemap.grid, pos);
	editor_touch *tile_touch;

	switch (current_mode) {
	case MODE_OBJECTS:
		data.object_ins = instance_at_pos(pos, OBJECT_MARGIN/view_editor->zoom, CP_ALL_LAYERS, CP_NO_GROUP);
		if ((remove_tool && !data.object_ins) || contains_instance(data.object_ins)) {
			return 0;
		}
		touch_unique_id touch_id = finger_bind(finger->fingerId);
		if (touch_id != -1) {
			editor_touch *touch = add_touchdata(MODE_OBJECTS, touch_id, pos_view, cpvzero, data);
			if (data.object_ins) {
				//TODO don't allow more than one(or two for rotate/scale of instance) binding per instance!
				touch->game_offset = cpvsub(data.object_ins->body->p, pos);
				if (!remove_tool) {
					touch->data_obj_delete = is_deletable(touch->data.object_ins) && ((grid_i.yrow == -1) || (pos.x < (panel_offset + PANEL_WIDTH - view_editor->view_width/2)));
				}
			}
			return 1; /* consume event: no zoom/pan/rotate of scroller*/
		}
		break;
	case MODE_RESIZE:
		data.resize_mode = radius_at_pos(pos, WALL_MARGIN/view_editor->zoom);
		if (data.resize_mode != RESIZE_NONE) {
			touch_unique_id touch_id = finger_bind(finger->fingerId);
			add_touchdata(MODE_RESIZE, touch_id, pos_view, cpvzero, data);
			return 1;
		}
		break;
	case MODE_TILEMAP:
		tile_touch = contains_tile_finger();
		if (tile_touch) { /* release finger and give over to view */
			finger_unbind(tile_touch->ID);
			//llist_remove(ll_touches, tile_touch);
			//pool_release(pool_touches, tile_touch);
		} else if (grid_i.yrow != -1) {
			touch_unique_id touch_id = finger_bind(finger->fingerId);
			byte tile = lvl->tilemap.data[current_tlay][grid_i.yrow][grid_i.xcol];
			data.tile_mode = tile ? TILE_CLEAR : TILE_ADD;
			add_touchdata(MODE_TILEMAP, touch_id, pos_view, cpvzero, data);
			return 1;
		}
		break;
	}
	return 0;
}

static int touch_motion(SDL_TouchFingerEvent *finger)
{
	cpVect pos_view = cpv(finger->x, finger->y);
	editor_touch *touch = get_touch(finger->fingerId);

	switch(current_mode) {
	case MODE_OBJECTS:
		if (touch && touch->mode == MODE_OBJECTS) {
			touch->viewpos_cur = pos_view;
			if (!touch->data.object_ins && !remove_tool && (cpvdistsq(touch->viewpos_start, pos_view) > 20*20)) {
				fprintf(stderr, "DEBUG: motion unbind\n");
				finger_unbind(touch->ID);
				llist_remove(ll_touches, touch);
				pool_release(pool_touches, touch);
			}
			return 1;
		}
		return 0;
	case MODE_RESIZE:
		if (touch && touch->mode == MODE_RESIZE) {
			touch->viewpos_cur = pos_view;
			return 1;
		}
		break;
	case MODE_TILEMAP:
		if (touch && touch->mode == MODE_TILEMAP) {
			touch->viewpos_cur = pos_view;
			return 1;
		}
		break;
	}
	return 0;
}

static int touch_up(SDL_TouchFingerEvent *finger)
{
	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	editor_touch *touch = get_touch(finger->fingerId);
	grid_index grid_i = grid_getindex(lvl->tilemap.grid, pos);

	if (touch) {
		switch(current_mode) {
		case MODE_OBJECTS:
			if ((touch->mode == MODE_OBJECTS) && !remove_tool) {
				if (touch->data_obj_delete) {
					delete_instance(touch);
				} else if ((grid_i.yrow != -1) && touch->data.object_ins == NULL) {
					//TODO make sure there are no other instances beneath pos!
					instance_create(object_by_name(object_type), level_get_param(&(lvl->params), object_type, param_name), pos, cpvzero);
					llist_remove(ll_touches, touch);
					pool_release(pool_touches, touch);
				} else {
					return 0;
				}
				return 1;
			}
			break;
		case MODE_RESIZE:
			if (touch->mode == MODE_RESIZE) {
				llist_remove(ll_touches, touch);
				pool_release(pool_touches, touch);
				return 1;
			}
			break;
		case MODE_TILEMAP:
			if (touch->mode == MODE_TILEMAP) {
				llist_remove(ll_touches, touch);
				pool_release(pool_touches, touch);
				return 1;
			}
			break;
		}
	}
	return 0;
}

static int editor_drag_button(button btn_id, SDL_TouchFingerEvent *finger, void *drag_data)
{
	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	int index = *((int *)&drag_data);
	float offset = btn_xoffset[index] + finger->dx * view_editor->view_width;
	if (offset >= DRAG_LIMIT) {
		btn_xoffset[index] = 0;
		select_object_type(drag_data);
		finger_release(finger->fingerId);
		button_clear(btn_id);
		instance *ins = instance_create(object_by_name(object_type), level_get_param(&(lvl->params), object_type, param_name), pos, cpvzero);
		touch_down(finger);

		touch_unique_id touch_id = finger_get_touch_id(finger->fingerId);
		if (touch_id) {
			/* get editor_touch ptr from finger_id */
			llist_begin_loop(ll_touches);
			while (llist_hasnext(ll_touches)) {
				editor_touch *touch = (editor_touch *) llist_next(ll_touches);
				if (touch->ID == touch_id) {
					touch->data.object_ins = ins;
					break;
				}
			}
			llist_end_loop(ll_touches);

			return 1;
		}
	} else {
		offset = fmaxf(fminf(DRAG_LIMIT, offset), 0);
		btn_xoffset[index] = offset;
	}
	return 0;
}

static void draw_gui(view *v)
{
	touch_place(scr_objects,-GAME_WIDTH/2+PANEL_WIDTH/2+panel_offset,0);
	cpVect obj_offset = scroll_get_offset(scr_objects);
	//TODO use another view for objects_scroller
	int i = active_obj_index;
	float x = -GAME_WIDTH/2+PANEL_WIDTH/2 + panel_offset;
	float y = GAME_HEIGHT/2-200 + obj_offset.y - i * 250;

	switch(current_mode) {
	case MODE_RESIZE:
	case MODE_OBJECTS:
		draw_quad_patch_center(0, SPRITE_PLAYERGUN001, cpv(x,y), cpv(200,200), 20, 0);
		bmfont_center(FONT_COURIER, cpv(0,-v->view_height/2),1,"MODE: %d",(int)current_mode);
		draw_color4b(50,50,50,25);
		draw_box(RLAY_GUI_BACK,cpv(-GAME_WIDTH/2+panel_offset,-GAME_HEIGHT/2),cpv(PANEL_WIDTH, GAME_HEIGHT),0,0);
		break;
	case MODE_TILEMAP:
		bmfont_center(FONT_COURIER, cpv(0,-v->view_height/2),1,"col: %d, row: %d", grid_i_cur.xcol, grid_i_cur.yrow);
		break;
	}
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

	SPRITE_ID spr = sprite_link("starcross01");
	for(i = 0; i<400; i++){
        Color col1 = {255,255,255,0};
        Color col2 = {0,0,0,0};
		int layer =  11 + roundf((1-we_randf*we_randf)*(layers-1-11));
		float size = 50 + we_randf*300 - layer*4;
		//byte l = 255 - 200 * layer / layers;
		//col = {l,l,l,255};
		float rand_x = we_randf;
		float rand_y = we_randf;
		cpVect pos = cpvmult(cpv(rand_x-0.5,rand_y-0.5),40000);
		float f = minf(rand_x*0.6 + 0.4*rand_y*(0.9+0.1*rand_x), 1);
		//col2.g = 255*(f);
		//col2.b = 255*(1-f);
        col2 = draw_col_rainbow((f+0.5)*1536);
		if (we_randf < 0.1) {
			col2.a = 1;
			state_add_sprite(state_editor, layer, SPRITE_SPIKEBALL, size,size, pos, 0, col2);
		} else {
			col2.r = (255 + col2.r)/2;
			col2.g = (255 + col2.g)/2;
			col2.b = (255 + col2.b)/2;
			col2.a = 0;
			state_add_dualsprite(state_editor, layer, spr, pos, cpv(size,size), col1, col2);
		}
	}

	state_enable_objects(state_editor, 1);
	state_enable_objupdate(state_editor, 0);
	state_enable_physics(state_editor, 1);

	view_editor->GUI = draw_gui;

	cpSpaceSetGravity(current_space, cpv(0, 0));
	cpSpaceSetDamping(current_space, 0);

	ll_touches = llist_create();
	pool_touches = pool_create(sizeof(editor_touch));

	btn_space  = button_create(SPRITE_BTN_HOME, 0, "", GAME_WIDTH/2 - 120, GAME_HEIGHT * (0.90-0.5), 125, 125);
	btn_clear  = button_create(SPRITE_BTN_RETRY,0, "", GAME_WIDTH/2 - 120, GAME_HEIGHT * (0.75-0.5), 125, 125);
	btn_test   = button_create(SPRITE_BTN_NEXT, 0, "", GAME_WIDTH/2 - 120, GAME_HEIGHT * (0.60-0.5), 125, 125);
	btn_save   = button_create(SPRITE_COIN, 	0, "", GAME_WIDTH/2 - 120, GAME_HEIGHT * (0.45-0.5), 125, 125);

	btn_state_objects = button_create(SPRITE_SPIKEBALL, 0, "Objects", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 400, 125, 125);
	btn_state_resize  = button_create(SPRITE_SPIKEBALL, 0, "Resize",  GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 300, 125, 125);
	btn_state_tilemap = button_create(SPRITE_SPIKEBALL, 0, "Tilemap", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 200, 125, 125);

	btn_delete = button_create(SPRITE_SPIKEBALL, 0, "X", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 100, 125, 125);
	btn_layer  = button_create(NULL, 0, "1", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 100, 125, 125);

	button_set_click_callback(btn_state_objects, editor_setmode, MODE_OBJECTS);
	button_set_click_callback(btn_state_resize, editor_setmode,  MODE_RESIZE);
	button_set_click_callback(btn_state_tilemap, editor_setmode,  MODE_TILEMAP);
	button_set_click_callback(btn_layer, setlayer, btn_layer);

	button_set_click_callback(btn_space, statesystem_set_state, state_stations);
	button_set_click_callback(btn_clear, tap_clear_editor, 0);
	button_set_click_callback(btn_test, start_editor_level, 0); // TODO test level with this button
	button_set_click_callback(btn_save, save_level_to_file, 0);
	button_set_click_callback(btn_save, save_level_to_file, 0);
	button_set_click_callback(btn_delete, tap_delete_click, 0);

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
	state_register_touchable_view(view_editor, btn_delete);
	state_register_touchable_view(view_editor, btn_state_objects);
	state_register_touchable_view(view_editor, btn_state_resize);
	state_register_touchable_view(view_editor, btn_state_tilemap);
	state_register_touchable_view(view_editor, btn_layer);

	/* OBJECT BUTTON INIT */
	float size = 200;
	float x = -GAME_WIDTH/2+size*2;
	float y = GAME_HEIGHT/2-size*2;
	SDL_Scancode key = SDL_SCANCODE_1;
	for (i = 0; i < EDITOR_OBJECT_COUNT; i++, y -= 250, key++) {
		SPRITE_ID spr_id = sprite_link(sprite_names[i]);
		button btn = button_create(spr_id, 0, "", x, y, size, size);
		btn_xoffset[i] = 0;
		button_set_drag_callback(btn, editor_drag_button, *(int **)(&i));
		button_set_click_callback(btn, select_object_type, *(int **)(&i));
		button_set_hotkeys(btn, key <= SDL_SCANCODE_0 ? key : 0, 0);
		button_set_enlargement(btn, 1.1);
		state_register_touchable_view(view_editor, btn); //TODO use another view for these buttons
		btn_objects[i] = btn;
	}
	scr_world = scroll_create(0,0,GAME_WIDTH,GAME_HEIGHT, 0.9, 3000, 1, 1, 0); // max 4 000 gu / sec
	scr_objects = scroll_create(-GAME_WIDTH/2+150,0,300,GAME_HEIGHT,0.9,50,0,0,1);
	scroll_set_callback(scr_world, touch_down, touch_motion, touch_up);
	scroll_set_bounds(scr_objects, cpBBNew(0,-GAME_HEIGHT/2,0,EDITOR_OBJECT_COUNT * 200));
	state_register_touchable_view(view_editor, scr_objects);
	state_register_touchable_view(view_editor, scr_world);

	lvl = level_load(WAFFLE_ZIP, "empty");
	lvl->tilemap.render_layers[TLAY_BACKGROUND] = RLAY_BACK_FRONT;
	lvl->tilemap.render_layers[TLAY_SOLID] = RLAY_GAME_BACK;
	lvl->tilemap.render_layers[TLAY_OVERLAY] = RLAY_GAME_FRONT;
	tap_clear_editor(NULL);
	editor_setmode(MODE_RESIZE);
}

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
	currentlvl = lvl;
}

static void pre_update(void)
{
	float panelspeed = 400*dt / 0.1;
	if (remove_tool || current_mode == MODE_RESIZE || current_mode == MODE_TILEMAP) {
		panel_offset = panel_offset - panelspeed > -400 ? panel_offset - panelspeed : -400;
	} else {
		panel_offset = panel_offset + panelspeed < 0 ? panel_offset + panelspeed : 0;
	}

	view_editor->zoom = scroll_get_zoom(scr_world);
	view_update(view_editor, cpvneg(scroll_get_offset(scr_world)), scroll_get_rotation(scr_world));
	touch_place(scr_objects,-GAME_WIDTH/2+150+panel_offset,0);
	cpVect obj_offset = scroll_get_offset(scr_objects);
	//TODO use another view for objects_scroller
	int i;
	float x = -GAME_WIDTH/2+150 + panel_offset;
	float y = GAME_HEIGHT/2-200 + obj_offset.y;
	for (i = 0; i < EDITOR_OBJECT_COUNT; i++, y -= 250) {
		touch_place(btn_objects[i], x + btn_xoffset[i], y); // + btn_xoffset[i]
		if (btn_xoffset[i] > 0) {
			btn_xoffset[i] *= 0.995; //TODO detect when button is released
		}
	}

	we_bool resizing = WE_FALSE;
	llist_begin_loop(ll_touches);
	while (llist_hasnext(ll_touches)) {
		editor_touch *touch = (editor_touch *) llist_next(ll_touches);
		if (finger_status(touch->ID, -1)) {
			touch->time += dt;
			switch(touch->mode) {
			case MODE_OBJECTS:
				move_instance(touch);
				break;
			case MODE_TILEMAP:
				paint_tile(touch);
				break;
			case MODE_RESIZE:
				resizing = WE_TRUE;
				move_radius(touch);

				break;
			}
			touch->viewpos_prev = touch->viewpos_cur;
			/*
            if (touch->time >= TAP_TIMEOUT) {
            	fprintf(stderr, "DEBUG: TAP TIMEOUT\n");
                finger_unbind(touch->ID);
            }
			 */
		} else {
			/* remove inactive touches */
			llist_remove(ll_touches, touch);
			pool_release(pool_touches, touch);
		}
	}
	llist_end_loop(ll_touches);

	if (!resizing) {
		float target_ir = lvl->tilemap.grid->rad[lvl->tilemap.grid->inner_i];
		float target_or = lvl->tilemap.grid->rad[lvl->tilemap.grid->outer_i-1];
		lvl->inner_radius = (target_ir+lvl->inner_radius)/2;
		lvl->outer_radius = (target_or+lvl->outer_radius)/2;
	}
}

static void post_update(void)
{

}


void space_draw_deck(void);
static void draw(void)
{
	draw_color4f(1,1,1,1);
	//tilemap_render(RLAY_BACK_BACK, lvl->tiles);
	space_draw_deck();

	draw_color4f(0.5,0.5,0.5,1);
	if (0 && current_mode == MODE_TILEMAP) { // (current_mode == MODE_RESIZE)
		grid_draw(lvl->tilemap.grid, 0,  10 / current_view->zoom);
	}

	if (current_mode == MODE_RESIZE) {
		draw_color4f(0.2,0.6,0.2,0.2);
		float r1, r2;
		float margin = WALL_MARGIN / current_view->zoom;
		r1 = lvl->inner_radius, r2 = r1;;
		r1 -= margin; r2 += margin;
		draw_donut(RLAY_GAME_FRONT, cpvzero, r1 < 0 ? 0 : r1, r2);
		r1 = lvl->outer_radius, r2 = r1;
		r1 -= margin; r2 += margin;
		draw_donut(RLAY_GAME_FRONT, cpvzero, r1 < 0 ? 0 : r1, r2);
	}
	tilemap2_render(&lvl->tilemap);
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
