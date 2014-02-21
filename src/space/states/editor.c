#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "we_data.h"
#include "space.h"
#include "../level/spacelvl.h"
#include "textinput.h"
#include "levelscreen.h"


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
static grid_index grid_i_cur = {-1,-1, 0, 0};

/*********** LEVEL DATA ***********/
static spacelvl *lvl_tmpl = NULL;
static char level_name[32];


/*********** EDITOR STATE ***********/
typedef enum EDITOR_MODE {
	MODE_RESIZE,
	MODE_TILEMAP,
	MODE_OBJECTS
} editor_mode;
static const int MODE_TOGGLE = 0xFFFF;
static editor_mode current_mode = MODE_RESIZE;

static void editor_setmode(editor_mode state);

/*********** RESIZE_MODE ***********/
float inner_resize_margin = 0, outer_resize_margin = 0;
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

typedef enum TILE_BRUSH {
	TBRUSH_NORMAL,
	TBRUSH_DESTROYABLE
} TILE_BRUSH;
static TILE_BRUSH tilebrush = TBRUSH_NORMAL;

/*********** OBJECT MODE ***********/
static void select_object_type(void *index);
#define PANEL_WIDTH 300
#define EDITOR_OBJECT_COUNT 8
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
		"factory_v2",
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
static button btn_lvlname;
static button btn_state_toggle;
static button btn_layer, btn_tile_toggler;
static button btn_objects[EDITOR_OBJECT_COUNT]; //TODO add param buttons
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

static void update_level_name(){
	textinput_start(state_editor, level_name, "LEVEL NAME:", 4, 20);
}

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
	add_object_recipe_name(lvl_tmpl->ll_recipes, obj->TYPE->NAME, (char*)&(((struct instance_dummy *)obj)->params), obj->p_start,0);
}


static void save_level_to_file(void *unused)
{
	llist_clear(lvl_tmpl->ll_recipes);
	instance_iterate(update_instances, NULL);
	lvl_tmpl->tm.layers = TLAY_COUNT;
	strcpy(lvl_tmpl->name, level_name);
	spacelvl_write(lvl_tmpl);
	solarsystem_load_levels_from_folder(user_system);
	solarsystem_write_solar_file(user_system, "levels/userlevels.json");
}

static void start_editor_level(void *unused)
{
	save_level_to_file(NULL);
	SDL_Log("EDITOR: STARTING LEVEL FROM EDITOR");
	llist_clear(lvl_tmpl->ll_recipes);
	instance_iterate(update_instances, NULL);
	statesystem_set_state(state_space);
	space_init_level_from_level(lvl_tmpl);
}

static void enable_objlist(int enable)
{
	if (enable) {

	} else {

	}
}

static void tap_clear_editor(void *force)
{
	if (!force) {
		SDL_ShowSimpleMessageBox(0, "Fjerne alt?", "Sikker på at du vil fjerne alt?", NULL);
	}
	//TODO implement dialog box and get confirmation from user
	currentlvl = lvl_tmpl;
	objectsystem_clear();
	extern obj_player * space_create_player(int id);
	space_create_player(1);
	select_object_type(0);

	cpVect p = cpvzero;
	instance *player = instance_first(obj_id_player);
	if (player) p = player->body->p;
	view_editor->zoom = 1;
	view_update(view_editor, p, 0);
	tilemap_clear(&lvl_tmpl->tm);
}

static void editor_setmode(editor_mode state)
{
	we_bool visible;
	if (state == MODE_TOGGLE) {
		state = (current_mode == MODE_OBJECTS) ? MODE_TILEMAP : MODE_OBJECTS;
	}
	current_mode = state;

	/* MODE_OBJECTS */
	visible = (current_mode == MODE_OBJECTS);
	button_set_text(btn_state_toggle, visible ? "Gulv" : "Fiender");

	/* MODE_RESIZE */
	visible = (current_mode == MODE_RESIZE);
	//btn_layer->visible = visible;

	/* MODE_TILEMAP */
	visible = (current_mode == MODE_TILEMAP);
	btn_layer->visible = visible;
	btn_tile_toggler->visible = visible;
}

static void setlayer(button btn)
{
	current_tlay++;
	current_tlay = current_tlay >= TLAY_COUNT ? 0 : current_tlay;
	char number[2] = {('0'+(char)current_tlay), '\0'};
	button_set_text(btn, number);
}

static void tilemode_toggle(button btn)
{
	tilebrush = 1 - tilebrush;
	char str[2] = {tilebrush ? 'N' : 'X', '\0'};
	button_set_text(btn, str);
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
	if (lvl_tmpl->inner_radius - margin < l && l < lvl_tmpl->inner_radius + margin ) {
		fprintf(stderr, "DEBUG: RESIZE_INNER\n");
		return RESIZE_INNER;
	} else if (lvl_tmpl->outer_radius - margin < l && l < lvl_tmpl->outer_radius + margin ) {
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
		float new_r = lvl_tmpl->inner_radius + (cpvlength(pos) - cpvlength(prev));
		if(new_r >= MIN_INNER_RADIUS && new_r <= lvl_tmpl->outer_radius - MIN_RADIUS_OFFSET) {
			lvl_tmpl->inner_radius = new_r;
			grid_setregion2f(lvl_tmpl->tm.grid, lvl_tmpl->inner_radius, lvl_tmpl->outer_radius);
			tilemap_updaterow(&lvl_tmpl->tm, lvl_tmpl->tm.grid->pol.inner_i);
			tilemap_updaterow(&lvl_tmpl->tm, lvl_tmpl->tm.grid->pol.inner_i+1);
		}
		inner_resize_margin += (1 - inner_resize_margin) / 2;
	} else if(touch->data.resize_mode == RESIZE_OUTER) {
		float new_r = lvl_tmpl->outer_radius + (cpvlength(pos) - cpvlength(prev));
		if(new_r <= MAX_OUTER_RADIUS && new_r >= lvl_tmpl->inner_radius + MIN_RADIUS_OFFSET) {
			lvl_tmpl->outer_radius = new_r;
			grid_setregion2f(lvl_tmpl->tm.grid, lvl_tmpl->inner_radius, lvl_tmpl->outer_radius);
			tilemap_updaterow(&lvl_tmpl->tm, lvl_tmpl->tm.grid->pol.outer_i-3);
			tilemap_updaterow(&lvl_tmpl->tm, lvl_tmpl->tm.grid->pol.outer_i-2);
			tilemap_updaterow(&lvl_tmpl->tm, lvl_tmpl->tm.grid->pol.outer_i-1);
			tilemap_updaterow(&lvl_tmpl->tm, lvl_tmpl->tm.grid->pol.outer_i);
		}
		outer_resize_margin += (1 - outer_resize_margin) / 2;
	}
}

static void paint_tile(editor_touch *touch)
{
	cpVect pos = view_view2world(view_editor, touch->viewpos_cur);
	grid_index grid_i = grid_getindex(lvl_tmpl->tm.grid, pos);
	grid_i_cur = grid_i;
	if (grid_i.yrow != -1) {
		if (touch->data.tile_mode == TILE_ADD) {
			tilemap_settile(&lvl_tmpl->tm, current_tlay, grid_i.xcol, grid_i.yrow, 1, tilebrush == TBRUSH_DESTROYABLE);

			if (current_tlay == TLAY_SOLID && !lvl_tmpl->tm.metadata[grid_i.yrow][grid_i.xcol].block) {
				cpVect verts[4];
				grid_getquad8cpv(lvl_tmpl->tm.grid, verts, grid_i.xcol, grid_i.yrow);
				cpShape *shape = cpPolyShapeNew(current_space->staticBody, 4, verts, cpvzero);
				lvl_tmpl->tm.metadata[grid_i.yrow][grid_i.xcol].block = shape;
				cpSpaceAddStaticShape(current_space, shape);
			}
		} else if (touch->data.tile_mode == TILE_CLEAR) {
			tilemap_settile(&lvl_tmpl->tm, current_tlay, grid_i.xcol, grid_i.yrow, 0, 0);

			if (current_tlay == TLAY_SOLID && lvl_tmpl->tm.metadata[grid_i.yrow][grid_i.xcol].block) {
				cpSpaceRemoveShape(current_space, lvl_tmpl->tm.metadata[grid_i.yrow][grid_i.xcol].block);
				cpfree(lvl_tmpl->tm.metadata[grid_i.yrow][grid_i.xcol].block);
				lvl_tmpl->tm.metadata[grid_i.yrow][grid_i.xcol].block = NULL;
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
		// check if pos is inside station walls
		grid_index grid_i = grid_getindex(lvl_tmpl->tm.grid,pos);
		if ((grid_i.yrow == -1) || (touch->viewpos_cur.x < (panel_offset + PANEL_WIDTH - view_editor->view_width/2))) {
			if (is_deletable(touch->data.object_ins)) {
				/* Jiggles instance to indicate that it will get removed on touch_up */
				pos = cpvadd(pos, cpvmult(cpv(we_randf-0.5,we_randf-0.5), 20));
				touch->data_obj_delete = WE_TRUE;
			} else if (grid_i.yrow == -1) {
				/* making sure non-deletable instances are not moved outside of station */
				float new_rad = grid_inner_radius(lvl_tmpl->tm.grid);
				new_rad = (grid_i.dist_sq <= new_rad*new_rad) ? new_rad : grid_outer_radius(lvl_tmpl->tm.grid);
				pos = WE_P2C(new_rad, grid_i.angle);
			}
		} else {
			touch->data_obj_delete = WE_FALSE;
		}
		touch->data.object_ins->p_start = cpvadd(pos, touch->game_offset);
		touch->data.object_ins->TYPE->call.init(touch->data.object_ins);
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
	grid_index grid_i = grid_getindex(lvl_tmpl->tm.grid, pos);
	editor_touch *tile_touch;

	switch (current_mode) {
	case MODE_OBJECTS: case MODE_RESIZE:
		data.object_ins = instance_at_pos(pos, OBJECT_MARGIN/view_editor->zoom, CP_ALL_LAYERS, CP_NO_GROUP);

		if (!data.object_ins) {
			data.resize_mode = radius_at_pos(pos, WALL_MARGIN/view_editor->zoom);
			if (data.resize_mode != RESIZE_NONE) {
				touch_unique_id touch_id = finger_bind(finger->fingerId);
				add_touchdata(MODE_RESIZE, touch_id, pos_view, cpvzero, data);
				return 1;
			}
			if (grid_i.yrow != -1) {
				byte tile = lvl_tmpl->tm.data[current_tlay][grid_i.yrow][grid_i.xcol];
				if (tile != TILE_TYPE_NONE) {
					editor_setmode(MODE_TILEMAP);
					return 0;
				}
			} else {
				editor_setmode(MODE_TILEMAP);
				return 0;
			}
		}

		if (!contains_instance(data.object_ins)) {
			touch_unique_id touch_id = finger_bind(finger->fingerId);
			if (touch_id != -1) {
				editor_touch *touch = add_touchdata(MODE_OBJECTS, touch_id, pos_view, cpvzero, data);
				if (data.object_ins) {
					//TODO don't allow more than one(or two for rotate/scale of instance) binding per instance!
					touch->game_offset = cpvsub(data.object_ins->body->p, pos);
					touch->data_obj_delete = is_deletable(touch->data.object_ins) && ((grid_i.yrow == -1) || (pos.x < (panel_offset + PANEL_WIDTH - view_editor->view_width/2)));
				}
				return 1; /* consume event: no zoom/pan/rotate of scroller*/
			}
		}
		break;
	case MODE_TILEMAP:
		/* toggle to object mode? */
		data.object_ins = instance_at_pos(pos, OBJECT_MARGIN/view_editor->zoom, CP_ALL_LAYERS, CP_NO_GROUP);
		if (!contains_instance(data.object_ins)) {
			if (data.object_ins) {
				touch_unique_id touch_id = finger_bind(finger->fingerId);
				editor_touch *touch = add_touchdata(MODE_OBJECTS, touch_id, pos_view, cpvzero, data);
				touch->game_offset = cpvsub(data.object_ins->body->p, pos);
				touch->data_obj_delete = is_deletable(touch->data.object_ins) && ((grid_i.yrow == -1) || (pos.x < (panel_offset + PANEL_WIDTH - view_editor->view_width/2)));
				editor_setmode(MODE_OBJECTS);
				return 1;
			}
		}

		tile_touch = contains_tile_finger();
		if (tile_touch) { /* release finger and give over to view */
			finger_unbind(tile_touch->ID);
			//llist_remove(ll_touches, tile_touch);
			//pool_release(pool_touches, tile_touch);
		} else if (grid_i.yrow != -1) {
			touch_unique_id touch_id = finger_bind(finger->fingerId);
			byte tile = lvl_tmpl->tm.data[current_tlay][grid_i.yrow][grid_i.xcol];
			data.tile_mode = tile ? TILE_CLEAR : TILE_ADD;
			add_touchdata(MODE_TILEMAP, touch_id, pos_view, cpvzero, data);
			return 1;
		}

		data.resize_mode = radius_at_pos(pos, WALL_MARGIN/view_editor->zoom);
		if (data.resize_mode != RESIZE_NONE) {
			touch_unique_id touch_id = finger_bind(finger->fingerId);
			add_touchdata(MODE_RESIZE, touch_id, pos_view, cpvzero, data);
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
	if (touch) {
		switch(touch->mode) {
		case MODE_OBJECTS:
			touch->viewpos_cur = pos_view;
			if (!touch->data.object_ins && (cpvdistsq(touch->viewpos_start, pos_view) > 20*20)) {
				fprintf(stderr, "DEBUG: motion unbind\n");
				finger_unbind(touch->ID);
				llist_remove(ll_touches, touch);
				pool_release(pool_touches, touch);
			}
			return 1;
		case MODE_RESIZE:
			/* NO BREAK */
		case MODE_TILEMAP:
			touch->viewpos_cur = pos_view;
			return 1;
		}
	}
	return 0;
}

static int touch_up(SDL_TouchFingerEvent *finger)
{
	cpVect pos_view = cpv(finger->x, finger->y);
	cpVect pos = view_view2world(view_editor, pos_view);
	editor_touch *touch = get_touch(finger->fingerId);
	grid_index grid_i = grid_getindex(lvl_tmpl->tm.grid, pos);

	if (touch) {
		switch(touch->mode) {
		case MODE_OBJECTS:
			if (touch->data_obj_delete) {
				delete_instance(touch);
			} else if ((grid_i.yrow != -1) && touch->data.object_ins == NULL) {
				//TODO make sure there are no other instances beneath pos!
				instance_create(object_by_name(object_type), param_get(object_type, param_name), pos, cpvzero);
				llist_remove(ll_touches, touch);
				pool_release(pool_touches, touch);
			} else {
				return 0;
			}
			/* NO BREAK */
		case MODE_RESIZE:
			/* NO BREAK */
		case MODE_TILEMAP:
			llist_remove(ll_touches, touch);
			pool_release(pool_touches, touch);
			return 1;
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
		instance *ins = instance_create(object_by_name(object_type), param_get(object_type, param_name), pos, cpvzero);
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

static Color tilecol = {31,143,31,200};
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
	btn_lvlname   = button_create(0, 	0, level_name, 0, GAME_HEIGHT/2 - 50, 125, 125);

	btn_state_toggle = button_create(SPRITE_WHITE, 0, "", GAME_WIDTH/2 - 200, GAME_HEIGHT * (0.2-0.5), 320, 100);
	btn_layer  = button_create(NULL, 0, "", GAME_WIDTH/2 - 25, -GAME_HEIGHT/2 + 25, 50, 50);
	btn_tile_toggler  = button_create(NULL, 0, "X", GAME_WIDTH/2 - 200, -GAME_HEIGHT/2 + 50, 100, 100);

	button_set_backcolor(btn_state_toggle, tilecol);

	button_set_click_callback(btn_state_toggle, (btn_click_callback) editor_setmode, MODE_TOGGLE);
	button_set_click_callback(btn_layer, (btn_click_callback) setlayer, btn_layer);
	button_set_click_callback(btn_tile_toggler, (btn_click_callback) tilemode_toggle, btn_tile_toggler);
	button_set_click_callback(btn_lvlname, update_level_name, 0);

	button_set_click_callback(btn_space, statesystem_set_state, state_stations);
	button_set_click_callback(btn_clear, tap_clear_editor, 0);
	button_set_click_callback(btn_test, statesystem_set_state, state_space);
	button_set_click_callback(btn_save, save_level_to_file, 0);

	button_set_hotkeys(btn_state_toggle, SDL_SCANCODE_1, 0);

	button_set_hotkeys(btn_test, KEY_RETURN_1, KEY_RETURN_2);
	button_set_hotkeys(btn_space, KEY_ESCAPE, 0);

	button_set_enlargement(btn_space, 1.5);
	button_set_enlargement(btn_clear, 1.5);
	button_set_enlargement(btn_test, 1.5);
	button_set_enlargement(btn_save, 1.5);
	button_set_enlargement(btn_state_toggle, 1.2);

	state_register_touchable_view(view_editor, btn_space);
	state_register_touchable_view(view_editor, btn_clear);
	state_register_touchable_view(view_editor, btn_test);
	state_register_touchable_view(view_editor, btn_save);
	state_register_touchable_view(view_editor, btn_state_toggle);
	state_register_touchable_view(view_editor, btn_layer);
	state_register_touchable_view(view_editor, btn_tile_toggler);
	state_register_touchable_view(view_editor, btn_lvlname);

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

	lvl_tmpl = spacelvl_parse(WAFFLE_ZIP, "empty");
	tap_clear_editor(WE_TRUE);
}

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void pre_update(void)
{
	button_set_text(btn_lvlname, level_name);
	float panelspeed = 400*dt / 0.1;
	if (current_mode == MODE_TILEMAP) {
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

	we_bool resize_inn = WE_FALSE, resize_out = WE_FALSE;
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
				resize_inn = touch->data.resize_mode == RESIZE_INNER ? WE_TRUE : resize_inn;
				resize_out = touch->data.resize_mode == RESIZE_OUTER ? WE_TRUE : resize_out;
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

	if (!resize_inn) {
		inner_resize_margin /= 2;
	}
	if (!resize_out) {
		outer_resize_margin /= 2;
	}
	if (!(resize_inn || resize_out)) {
		float target_ir = lvl_tmpl->tm.grid->pol.rad[lvl_tmpl->tm.grid->pol.inner_i];
		float target_or = lvl_tmpl->tm.grid->pol.rad[lvl_tmpl->tm.grid->pol.outer_i-1];
		lvl_tmpl->inner_radius = (target_ir+lvl_tmpl->inner_radius)/2;
		lvl_tmpl->outer_radius = (target_or+lvl_tmpl->outer_radius)/2;
		lvl_tmpl->height = lvl_tmpl->outer_radius - lvl_tmpl->inner_radius;
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

	draw_color4b(80,80,80,100);
	if (current_mode == MODE_TILEMAP) {
		grid_draw(lvl_tmpl->tm.grid, 0,  6 / current_view->zoom);
	}

	draw_color4f(0.2,0.6,0.2,0.2);
	float r1, r2, margin;
	margin = 20 * inner_resize_margin / current_view->zoom;
	r1 = lvl_tmpl->inner_radius, r2 = r1;;
	r1 -= margin; r2 += margin;
	if (margin > 0.5) draw_donut(RLAY_GAME_FRONT, cpvzero, r1 < 0 ? 0 : r1, r2);
	margin = 20 * outer_resize_margin / current_view->zoom;
	r1 = lvl_tmpl->outer_radius, r2 = r1;
	r1 -= margin; r2 += margin;

	if (margin > 0.5) draw_donut(RLAY_GAME_FRONT, cpvzero, r1 < 0 ? 0 : r1, r2);
	tilemap2_render(&lvl_tmpl->tm);
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
	//TODO perhaps save lvl when user is exiting app?
}

static void on_enter(STATE_ID state_prev)
{
	if (state_prev == state_levelscreen) {
		lvl_tmpl = get_current_lvl_template();
		currentlvl = lvl_tmpl;
		objectsystem_clear();
		spacelvl_load2state(lvl_tmpl);
		editor_setmode(MODE_OBJECTS);
	}
}


static void on_leave(STATE_ID state_next)
{
	if (state_next == state_stations) {
		//TODO clear editor
	}
}

static void destroy(void)
{
	objectsystem_clear();
	llist_destroy(ll_touches);
	pool_destroy(pool_touches);
}

