#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "statesystem.h"
#include "../data/llist.h"
#include "../input/touch.h"
#include "../engine.h"
#include "../debug/monitor.h"
#include "we_utils.h"

#include "SDL.h"
#define MAX_INNER_STATES 10

int debug_disable_render = 0;

LList ll_states;

cpSpace *current_space;
object_system *current_objects;
particle_system *current_particles;
view * current_view;

static cpFloat phys_step = 1/60.0f;
//#define phys_step 0.01666666667f
static float accumulator = 0;

typedef struct systemstate State;
struct systemstate {
	char name[32];
    State *prev;
    State *next;

    STATE_ID id;

    int inner_states;
    int current_inner_state;
    float time_alive;
    float time_in_inner_state;

    LList cameras;

    LList touch_objects;

    int disable_updates;
    int disable_physics;
    int objects_enabled;
    int particles_enabled;

    object_system *objects;
    particle_system *particles;
    layer_system *layersystem;

    void (*inner_update[MAX_INNER_STATES])();
    void (*inner_draw[MAX_INNER_STATES])();

    state_funcs call;
};

static STATE_ID state_beeing_rendered = NULL;

State *stack_head = NULL;
State *stack_tail = NULL;

static void statesystem_free(STATE_ID state_id);

void statesystem_init()
{
	ll_states = llist_create();
	llist_set_remove_callback(ll_states, statesystem_free);
}

static void update_global_current_var(State *state)
{
	if (state->id != state) {
		fprintf(stderr, "ERROR, state invalid\n");
		exit(-1);
	}

	current_particles = state->particles;
    current_objects = state->objects;
	if (current_objects)
		current_space = current_objects->space;
	else
		current_space = NULL;
	//fprintf(stderr, "DEBUG: par=%p, obj=%p, spc=%p\n",current_particles, current_objects, current_space);
}

STATE_ID statesystem_create_state(const char *name, int inner_states, state_funcs *funcs)
{
	State *state = calloc(1, sizeof *state);
	strncpy(state->name, name, 32);
	fprintf(stderr, "INFO: Statesystem: created %s\n", state);
    state->id = state;

    state->cameras = llist_create();
    llist_set_remove_callback(state->cameras, (ll_rm_callback) view_free);
    state_view_add(state);
    state->touch_objects = llist_create();
    state->inner_states = inner_states;
	if (inner_states > MAX_INNER_STATES) {
		SDL_Log("Number of inner states(%d) exceeded MAX_INNER_STATES (%d)\n", inner_states, MAX_INNER_STATES);
		exit(-1);
	}

    llist_add(ll_states, (void*)state);
    state->call = *funcs;
    state->objects = NULL;
    state->particles = NULL;
    state->layersystem = layersystem_new();


    update_global_current_var(state);

    return state->id;
}

void state_enable_objects(STATE_ID state_id, int enabled)
{
	State *state = (State *) state_id;
	if (state->objects == NULL && enabled) {
		state->objects = objectsystem_new();
		current_objects = state->objects;
		current_space = state->objects->space;
	}
	state->objects_enabled = enabled;
}

void state_enable_physics(STATE_ID state_id, int enabled)
{
	State *state = (State *) state_id;
	if (state->objects) {
		state->disable_physics = !enabled;
	}
}

void state_enable_objupdate(STATE_ID state_id, int enabled)
{
	State *state = (State *) state_id;
	if (state->objects) {
		state->disable_updates = !enabled;
	}
}

void state_enable_particles(STATE_ID state_id, int enabled)
{
	State *state = (State *) state_id;
	if (state->particles == NULL && enabled) {
		state->particles = particlesystem_new();
		state->particles->state = state;
		current_particles = state->particles;
	}
	state->particles_enabled = enabled;
}


view *state_view_add(STATE_ID state_id)
{
	State *state = (State *) state_id;
	view *v = view_new();
	llist_add(state->cameras, (void *)v);
	return v;
}

view *state_view_get(STATE_ID state_id, int index)
{
	State *state = (State *) state_id;
	return (view *) llist_at_index(state->cameras, index);
}

void state_view_enable(STATE_ID state_id, int index, int enabled)
{
	State *state = (State *) state_id;
	((view *) llist_at_index(state->cameras, index))->enabled = enabled;
}


void state_add_inner_state(STATE_ID state_id, int inner_state, void (*update)(), void (*draw)())
{
	State *state = (State *) state_id;

    state->inner_update[inner_state] = update;
    state->inner_draw[inner_state] = draw;
}

void state_set_inner_state(STATE_ID state_id, int inner_state)
{
	State *state = (State *) state_id;

    state->time_in_inner_state = 0;
    state->current_inner_state = inner_state;
}

static void on_leave(State *state, STATE_ID next)
{
    fprintf(stderr, "DEBUG: ->| on_leave %s \t-> %s;\n", state, next);
    state->call.on_leave(next);
}
static void on_enter(State *state, STATE_ID prev)
{
    fprintf(stderr, "DEBUG: |-> on_enter %s \t<- %s;\n", state, prev);
    state->call.on_enter(prev);
}

void statesystem_push_state(STATE_ID state_id)
{
	State *state = (State *) state_id;
	STATE_ID prev = (STATE_ID) stack_head;
	STATE_ID next = state_id;

    state->time_alive = 0;
    stack_head->next = state;
    stack_head->next->prev = stack_head;
	update_global_current_var(stack_head);
	on_leave(stack_head, next);
    stack_head = stack_head->next;
	update_global_current_var(stack_head);
	on_enter(stack_head, prev);
}

void statesystem_pop_state(void *unused)
{
    State *temp = stack_head;
	STATE_ID prev = (STATE_ID) stack_head;
	STATE_ID next = stack_head->prev;
	if (!next) {
		SDL_Log("ERROR: statesystem.c trying to pop last state");
		return;
	}

	update_global_current_var(stack_head);
	on_leave(stack_head, next);
    stack_head = next;
    temp->prev = NULL;
    stack_head->next = NULL;
	update_global_current_var(stack_head);
	on_enter(stack_head, prev);
}

void statesystem_set_state(STATE_ID state_id)
{
	State *stack_next, *state = stack_tail;

	STATE_ID prev = (STATE_ID) stack_head;
	STATE_ID next = state_id;

	while (state) {
		state->prev = NULL;
		stack_next = state->next;
		state->next = NULL;
		update_global_current_var(state);
		on_leave(state, next);
		state = stack_next;
	}

    if (stack_head != state_id) {
    	finger_release_all(); /* clear all bindings */
    }

	state = (State *) state_id;
    state->time_alive = 0;
    stack_head = state;
    stack_tail = state;
    update_global_current_var(state);
	on_enter(state, prev);
}

static void update_instances(instance *obj, void *data)
{
	if(obj->alive){
		instance_update(obj);
	}
}

void statesystem_update(void)
{
	current_view = NULL;
	/* state pre-update */
	State *state = stack_head;

	update_global_current_var(state);
	if (state->call.pre_update) {
		state->call.pre_update();
	}

	/* UPDATE IN-GAME TOUCHABLES */
	LList game_touchies = state->touch_objects;
	llist_begin_loop(game_touchies);
	while(llist_hasnext(game_touchies)) {
		void *touchy = llist_next(game_touchies);
		((touch_calls *) (*(void **)touchy))->update(touchy);
	}
	llist_end_loop(game_touchies);

	/* UPDATE HUD TOUCHABLES */
	llist_begin_loop(state->cameras);
	while(llist_hasnext(state->cameras)) {
		view * cam = llist_next(state->cameras);
		if (!cam->enabled)
			continue;
		LList view_touchies = cam->touch_objects;
		llist_begin_loop(view_touchies);
		while(llist_hasnext(view_touchies)) {
			void *touchy = llist_next(view_touchies);
			((touch_calls *) (*(void **)touchy))->update(touchy);
		}
		llist_end_loop(view_touchies);
	}
	llist_end_loop(state->cameras);


	/* inner state update */
	if (state->inner_states > 0
			&& state->inner_update[state->current_inner_state]) {
		state->inner_update[state->current_inner_state]();
	}

	/* update objects */
	if (current_objects && current_space) {
		if (state->objects_enabled) {
			if (!state->disable_updates)
				instance_iterate(update_instances, NULL);

			/* update Chipmunk */
			//TODO check object system
			if (!state->disable_physics) {
				accumulator += dt;
				while (accumulator >= phys_step) {
					cpSpaceStep(current_space, phys_step);
					accumulator -= phys_step;
				}
			}
		} else {
			cpSpaceStep(current_space, 0);
		}
	}

	/* update particle system */
	if (state->particles_enabled && current_particles) {
		particles_update(current_particles);
	}

	/* state post update */
	if (state->call.post_update) {
		state->call.post_update();
	}

	/* update all lists and remove all dead objects */
	if (state->objects_enabled && current_objects) {
		void instance_poststep(void);
		instance_poststep();
	}
}

static void render_instances(instance *obj, void *data)
{
	instance_render(obj);
}

void view_clip(view *cam);
void view_transform2view(view *cam);
void view_transform2port(view *cam);

extern int ELEMENT_APPEND_COUNT;
extern int ELEMENT_APPEND_ACTUAL_COUNT;

void statesystem_draw(void)
{
	ELEMENT_APPEND_COUNT = 0;
	ELEMENT_APPEND_ACTUAL_COUNT = 0;

	/* render all states in stack */
    State *state = stack_tail;
    while(state){
    	update_global_current_var(state);
    	/* render current state */
    	state_beeing_rendered = state->id;
    	draw_push_matrix();
    	llist_begin_loop(state->cameras);
    	while(llist_hasnext(state->cameras)) {
    		view * cam = llist_next(state->cameras);
    		if (!cam->enabled)
    			continue;

    		view_clip(cam);
    		view_transform2view(cam);
    		draw_push_matrix();

    		if (state->call.draw) {
    			state->call.draw();
    		}
    		draw_pop_matrix();

    		draw_push_matrix();
    		/* draw all objects */
    		if (current_objects) {
    			if (!debug_disable_render)
    				instance_iterate(render_instances, NULL);
    			debugdraw_space(current_space);
    		}
    		draw_pop_matrix();

    		draw_push_matrix();
    		if (state->particles_enabled && current_particles)  {
    			particles_draw(current_particles); //TODO render from layers
    		}
    		draw_pop_matrix();

    		draw_push_matrix();
    		/* render inner state */ //TODO check if working?
    		if(state->inner_states > 0 &&
    				state->inner_draw[state->current_inner_state]){
    			state->inner_draw[state->current_inner_state]();
    		}
    		draw_pop_matrix();


    		/* render in-game touchables */
    		LList state_touchies = state->touch_objects;
    		llist_begin_loop(state_touchies);
    		while(llist_hasnext(state_touchies)) {
    			touchable *touchy = llist_next(state_touchies);
    			if (touchy->visible) {
    				touchy->calls->render(touchy);
    			}
    		}
    		llist_end_loop(state_touchies);

    		/* RENDER HUD */
    		view_transform2port(cam);
    		draw_push_matrix();
    		draw_push_matrix();
    		extern int debug_draw;
    		if(keys[SDL_SCANCODE_MINUS]) {
    		        debug_draw = !debug_draw;
    		        keys[SDL_SCANCODE_MINUS] = 0;
    		}
    		if (debug_draw) {
    		    if(cam) monitor_render(cam);
    			//draw_color_rgbmulta4f(1,0,1,0.1);
    			//draw_box(0, cpvzero, cpv(cam->view_width-10, cam->view_height-10), 0, 1);
    			draw_color_rgbmulta4f(0,1,0,1.0);
    			draw_box(0, cpvzero, cpv(10,10), 0, 1);
    		}

    		draw_pop_matrix();
    		if (cam->GUI) {
    			cam->GUI(cam);
    		}
    		draw_pop_matrix();

    		/* RENDER HUD-touchables */
    		LList cam_touchies = cam->touch_objects;
    		llist_begin_loop(cam_touchies);
    		while(llist_hasnext(cam_touchies)) {
    			touchable *touchy = llist_next(cam_touchies);
    			if (touchy->visible) {
    				touchy->calls->render(touchy);
    			}
    		}
    		llist_end_loop(cam_touchies);

    		layersystem_render(state, cam);
    		current_view = NULL;
    	}
    	llist_end_loop(state->cameras);

    	draw_pop_matrix();
    	state = state->next;
    }

    state_beeing_rendered = NULL;

    static int counter = 0;
    if (++counter >= 60) {
    	extern int TMP_DRAW_CALLS;
#if !ARCADE_MODE
    	//fprintf(stderr, "RENDER_INFO: %d of %d elements rendered in %d draw calls\n", ELEMENT_APPEND_ACTUAL_COUNT, ELEMENT_APPEND_COUNT, (TMP_DRAW_CALLS+30)/60);
#endif
    	counter = 0;
    	TMP_DRAW_CALLS = 0;
    }
}

void statesystem_pause(void)
{
	if (stack_head->call.on_pause) {
		stack_head->call.on_pause();
	}
}

static void statesystem_free(STATE_ID state_id)
{
	State *state = (State *) state_id;
	update_global_current_var(state);

	if(state->call.destroy){
		state->call.destroy();
	}
	if(state->inner_states > 0){
		// free(state->inner_update);
		//  free(state->inner_draw);
	}

	//free object and particle systems
	objectsystem_free(state->objects);
	particlesystem_free(state->particles);
	state->particles = NULL;
	state->objects = NULL;
	update_global_current_var(state);

	//free buttons and other touchable objects
	llist_destroy(state->touch_objects);

	//free cameras
	llist_destroy(state->cameras);
}

void statesystem_destroy(void)
{
	llist_destroy(ll_states);
}

static cpVect game_p, port_p, view_p;

//TODO CLEAN UP THIS MESSY FUNCTION!
void statesystem_push_event(SDL_Event *event)
{
	static Uint32 latest_motion_event = 0;
	int consumed = 0;
	LList game_touchies = stack_head->touch_objects;
	float tx = event->tfinger.x, ty = event->tfinger.y;

	//TODO transform touch to in-game coords
	if (stack_head->call.sdl_event) {
		consumed |= stack_head->call.sdl_event(event); //todo add touch callback to scroll(or all touch events, through general functions) with the possibility to ignore ordinary touch event?
	}
	if (consumed) return;

	// get the top most view beneath the touch location //TODO go through all views as long there is no response?

	view *touched_view = NULL;
	int view_index = 0x1FED;
	LList view_touchies = NULL;
	llist_begin_loop(stack_head->cameras);
	if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
		port_p = cpv(tx * WINDOW_WIDTH, (1-ty) * WINDOW_HEIGHT);
		game_p = cpv((tx  - 0.5) * GAME_WIDTH, (0.5 - ty) * GAME_HEIGHT);
		int index = 0;
		while(llist_hasnext(stack_head->cameras)) {
			view *cam = llist_next(stack_head->cameras);
			if (!cam->enabled) {
				++index;
				continue;
			}

			if (cpBBContainsVect(cam->priv_port_box, port_p)) {
			 	view_touchies = cam->touch_objects;

			 	// get normalized view position and dimension and compute local view touch
			 	float n_px = cam->port_pos.x / WINDOW_WIDTH;
			 	float n_py = cam->port_pos.y / WINDOW_HEIGHT;
			 	float p_pw = cam->port_width / WINDOW_WIDTH;
			 	float p_ph = cam->port_height / WINDOW_HEIGHT;
			 	float ptx = ((tx-n_px) / p_pw - 0.5f) * GAME_WIDTH;
			 	float pty = (0.5f - (ty-n_py) / p_ph) * GAME_HEIGHT;

			 	view_p = matrix2d_multcpv(cam->priv_port_invtransform, cpv(ptx,pty));
			 	event->tfinger.x = view_p.x;
			 	event->tfinger.y = view_p.y;
			 	view_index = index;
			 	touched_view = cam;
			}
			++index;

		}
	}
	llist_end_loop(stack_head->cameras);


	//TMP DEBUG
	// /*
	if (view_touchies) {
		if (keys[SDL_SCANCODE_Q]) {
			keys[SDL_SCANCODE_Q] = 0;
			view_set_orientation(touched_view, touched_view->port_orientation + 1);
		} else if (keys[SDL_SCANCODE_Z]) {
			fprintf(stderr, "Touch in view_%d    X: %-6.3f Y: %-6.3f\n", view_index, event->tfinger.x,event->tfinger.y);
		}
	}
	//*/
	game_p = view_view2world(touched_view, game_p);

	llist_begin_loop(game_touchies);
	switch(event->type) {
	case SDL_KEYDOWN:
		llist_begin_loop(stack_head->cameras);
		while(llist_hasnext(stack_head->cameras)) {
			view *cam = llist_next(stack_head->cameras);
			if (!cam->enabled) {
				continue;
			}

			LList view_touchies = cam->touch_objects;
			llist_begin_loop(view_touchies);
			while (!consumed && llist_hasnext(view_touchies)) {
				touchable *touchy = llist_next(view_touchies);
				if (touchy->enabled && touchy->visible) {
					if (touchy->calls->touch_keypress(touchy, event->key.keysym.scancode))
						consumed = 1;
				}
			}
			llist_end_loop(view_touchies);
		}
		llist_end_loop(stack_head->cameras);

		while(!consumed && llist_hasnext(game_touchies)) {
			touchable *touchy = llist_next(game_touchies);
			if (touchy->enabled && touchy->visible) {
				if (touchy->calls->touch_keypress(touchy, event->key.keysym.scancode))
					consumed = 1;
			}
		}
		break;
	case SDL_FINGERDOWN:
		if (view_touchies) {
			llist_begin_loop(view_touchies);
			while (!consumed && llist_hasnext(view_touchies)) {
				touchable *touchy = llist_next(view_touchies);
				if (touchy->enabled && touchy->visible) {
					if (touchy->calls->touch_down(touchy, &event->tfinger))
						consumed = 1;
				}
			}
			llist_end_loop(view_touchies);
		}
		event->tfinger.x = game_p.x;
		event->tfinger.y = game_p.y;
		while(!consumed && llist_hasnext(game_touchies)) {
			touchable *touchy = llist_next(game_touchies);
			if (touchy->enabled && touchy->visible) {
				if (touchy->calls->touch_down(touchy, &event->tfinger))
					consumed = 1;
			}
		}
		break;
	case SDL_FINGERMOTION:
		if (event->tfinger.timestamp < latest_motion_event) {
			fprintf(stderr, "WARNING: skipping motion event\n");
			break;
		}
		latest_motion_event = event->tfinger.timestamp;
		if (view_touchies) {
			llist_begin_loop(view_touchies);
			while (!consumed && llist_hasnext(view_touchies)) {
				touchable *touchy = llist_next(view_touchies);
				if (touchy->enabled && touchy->visible) {
					if (touchy->calls->touch_motion(touchy, &event->tfinger))
						consumed = 1;
				}
			}
			llist_end_loop(view_touchies);
		}
		event->tfinger.x = game_p.x;
		event->tfinger.y = game_p.y;
		while(!consumed && llist_hasnext(game_touchies)) {
			touchable *touchy = llist_next(game_touchies);
			if (touchy->enabled && touchy->visible) {
				if (touchy->calls->touch_motion(touchy, &event->tfinger))
					consumed = 1;
			}
		}
		break;
	case SDL_FINGERUP:
		if (view_touchies) {
			llist_begin_loop(view_touchies);
			while (!consumed && llist_hasnext(view_touchies)) {
				touchable *touchy = llist_next(view_touchies);
				if (touchy->enabled && touchy->visible) {
					if (touchy->calls->touch_up(touchy, &event->tfinger))
						consumed = 1;
				}
			}
			llist_end_loop(view_touchies);
		}
		event->tfinger.x = game_p.x;
		event->tfinger.y = game_p.y;
		while(!consumed && llist_hasnext(game_touchies)) {
			touchable *touchy = llist_next(game_touchies);
			if (touchy->enabled && touchy->visible) {
				if (touchy->calls->touch_up(touchy, &event->tfinger))
					consumed = 1;
			}
		}
		break;
	}
	llist_end_loop(game_touchies);
}


cpVect statesystem_last_touch_game(void)
{
	return game_p;
}
cpVect statesystem_last_touch_port(void)
{
	return port_p;
}
cpVect statesystem_last_touch_view(void)
{
	return view_p;
}


void statesystem_call_update(STATE_ID state_id)
{
	State *state = (State *) state_id;
		if (state->call.pre_update) {
			state->call.pre_update();
	}

	/*
	LList list = state->touch_objects;
	llist_begin_loop(list);
	while (llist_hasnext(list)) {
		void *touchy = llist_next(list);
		((touch_calls *) (*(void **) touchy))->update(touchy);
	}
	llist_end_loop(list);

	if (state->inner_states > 0
			&& state->inner_update[state->current_inner_state]) {
		state->inner_update[state->current_inner_state]();
	}
	*/

	if (state->call.post_update) {
		state->call.post_update();
	}
}

STATE_ID statesystem_get_render_state(void)
{
	return state_beeing_rendered;
}


/**
 * Registers an in-game touchable to state
 */
void state_register_touchable(STATE_ID state_id, touchable *touchable)
{
	State *state = (State *) state_id;
	llist_add(state->touch_objects, touchable);
	touchable->container = NULL;

}

int state_remove_touchable(STATE_ID state_id, touchable *touchable)
{
	State *state = (State *) state_id;
	return llist_remove(state->touch_objects, touchable);
}

/**
 * Registers an overlay touchable to view
 */
void state_register_touchable_view(view *cam, touchable *touchable)
{
	llist_add(cam->touch_objects, touchable);
	touchable->container = cam;
}

void state_remove_touchable_view(view *cam, touchable *touchable)
{
	llist_remove(cam->touch_objects, touchable);
}

//void state_register_sprite(STATE_ID state_id, int layer, sprite *spr)
//{
//	State *state = (State *) state_id;
//	layersystem_register_sprite(state->layersystem, layer, spr);
//}

int state_add_layer(STATE_ID state_id)
{
	State *state = (State *) state_id;
	return layersystem_add_layer(state->layersystem);
}

// returns last state index
int state_add_layers(STATE_ID state_id, int number)
{
	State *state = (State *) state_id;
	while(number >=1 ) {
		layersystem_add_layer(state->layersystem);
		number--;
	}
	return layersystem_add_layer(state->layersystem);
}

int state_layer_count(STATE_ID state_id)
{
	State *state = (State *) state_id;
	return state->layersystem->num_layers;
}

layer_system * state_get_layersystem(STATE_ID state_id)
{
	return ((State *) state_id)->layersystem;
}

