/*
 * statesystem.h
 *
 *  Created on: Mar 22, 2013
 *      Author: karlmka
 */

#ifndef STATESYSTEM_H_
#define STATESYSTEM_H_

#include "SDL_events.h"
#include "we_graphics.h"
#include "we_tween.h"
#include "chipmunk.h"
#include "../components/object.h"
#include "../graphics/particles.h"
#include "../input/touch.h"

extern cpSpace *current_space;
extern object_system *current_objects;
extern particle_system *current_particles;
extern tween_system *current_tween_system;
extern view * current_view;

typedef void * STATE_ID;

typedef struct {
    void (*on_enter)(STATE_ID state_prev);
    void (*pre_update)(void);
    void (*post_update)(void);
    void (*draw)(void);
    int (*sdl_event)(SDL_Event *event);
    void (*on_pause)(void);
    void (*on_leave)(STATE_ID state_next);
    void (*destroy)(void);
} state_funcs;


/**
 * standard functions
 */
void statesystem_init(void);
void statesystem_update(void);
void statesystem_draw(void);
void statesystem_pause(void);
void statesystem_destroy(void);

/**
 * statesystem functions
 */

STATE_ID statesystem_get_render_state(void);

STATE_ID statesystem_create_state(const char *name, int inner_states, state_funcs *funcs);

void state_enable_objects(STATE_ID state_id, int enabled);
void state_enable_physics(STATE_ID state_id, int enabled);
void state_enable_objupdate(STATE_ID state_id, int enabled);
void state_enable_particles(STATE_ID state_id, int enabled);
void state_enable_tween_system(STATE_ID state_id, int enabled);

view *state_view_add(STATE_ID state_id);
view *state_view_get(STATE_ID state_id, int index);
void state_view_enable(STATE_ID state_id, int index, int enabled);

void state_add_inner_state(STATE_ID state, int inner_state, void (*update)(void), void (*draw)(void));
void state_set_inner_state(STATE_ID state, int inner_state);

void statesystem_push_state(STATE_ID state);
void statesystem_pop_state(void *unused);
void statesystem_set_state(STATE_ID state);

void statesystem_call_update(STATE_ID state_id);
void statesystem_push_event(SDL_Event *event);

cpVect statesystem_last_touch_game(void);
cpVect statesystem_last_touch_port(void);
cpVect statesystem_last_touch_view(void);

int state_remove_touchable(STATE_ID state_id, touchable *touchable);
void state_register_touchable(STATE_ID state_id, touchable *touchable);
void state_register_touchable_view(view *cam, touchable *touchable);
void state_remove_touchable_view(view *cam, touchable *touchable);

//void state_register_sprite(STATE_ID state_id, int layer, sprite *spr);
int state_add_layer(STATE_ID state_id);
int state_add_layers(STATE_ID state_id, int number);
int state_layer_count(STATE_ID state_id);
void state_set_layer_offset(STATE_ID state_id, int layer, cpVect offset);
void state_set_layer_parallax(STATE_ID state_id, int layer, float factor, float zoom_factor);
void * state_add_drawable(STATE_ID state_id, int layer, draw_callback drawfunc, void *drawdata, int datasize);
drawbl_dualspr *state_add_dualsprite(STATE_ID state_id, int layer, SPRITE_ID spr_id, cpVect pos, cpVect size, Color col1, Color col2);
sprite_ext *state_add_sprite(STATE_ID state_id, int layer, SPRITE_ID id, float w, float h, cpVect p, float angle, Color col);

void layersystem_render(STATE_ID state_id, view *cam);

#endif /* STATESYSTEM_H_ */

