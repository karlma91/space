#include "../game.h"
#include "../spaceengine.h"
#include "we_defstate.h"
#include "space.h"

#define THIS_IS_A_TOUCH_OBJECT 1
#include "../../engine/input/touch.h"

STATE_ID state_editor;

touchable touch_window;

static obj_param_tank tmp_tank_param = {
	500,
	50
};


void editor_init()
{
	statesystem_register(state_editor,0);
	int i;
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

	touchable * tp = &touch_window;
	REGISTER_CALLS(tp);

	touch_place(&touch_window, 0, 0);
	state_register_touchable(state_editor, &touch_window);

}

/* * * * * * * * * *
 * state functions *
 * * * * * * * * * */

static void on_enter(void)
{
}

static void pre_update(void)
{
}

static void post_update(void)
{

}

static void draw(void)
{
	if(keys[SDL_SCANCODE_UP]){
		current_view->p.y -= 100*dt;
	}else if(keys[SDL_SCANCODE_DOWN]){
		current_view->p.y += 100*dt;
	}
	if(keys[SDL_SCANCODE_LEFT]){
		current_view->p.x -= 100*dt;
	}else if(keys[SDL_SCANCODE_RIGHT]){
		current_view->p.x += 100*dt;
	}

	if(keys[SDL_SCANCODE_PAGEUP]){
		current_view->zoom *= 1 + 1 * dt;
	} else if(keys[SDL_SCANCODE_PAGEDOWN]) {
		current_view->zoom *= 1/(1 + 1 * dt);
	}
}

static void sdl_event(SDL_Event *event)
{
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

static cpVect prev;
static int dragged;
static int touch_down(touchable *t, SDL_TouchFingerEvent *finger)
{
	float zoom = current_view->zoom;
	prev = camera_inv_vect_view2world(current_view, cpv(finger->x, finger->y));
	prev = cpvmult(prev, 1/zoom);
	dragged = 0;
}

static int touch_motion(touchable *t, SDL_TouchFingerEvent *finger)
{
	float zoom = current_view->zoom;
	cpVect pos = camera_inv_vect_view2world(current_view, cpv(finger->x, finger->y));
	pos = cpvmult(pos, 1/zoom);
	cpVect delta = cpvsub(prev, pos);
	prev = pos;
	cpVect vpos = current_view->p;
	view_update(current_view, cpvadd(vpos, delta), 0);
	dragged = 1;
}

static int touch_up(touchable *t, SDL_TouchFingerEvent *finger)
{
	cpVect pos = cpv(finger->x, finger->y);
	if(!dragged) {
		instance_create(obj_id_tank, &tmp_tank_param,pos, cpvzero);
	}
}

static int keypress_down(touchable *t, SDL_Scancode key)
{

}
