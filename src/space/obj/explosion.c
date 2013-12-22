
#include "object_types.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define OBJ_NAME explosion
#include "we_defobj.h"
#include "chipmunk_unsafe.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(RLAY_GAME_FRONT, explosion->param.em_expl, explosion->data.p_start);
	particles_get_emitter_at(RLAY_GAME_FRONT, explosion->param.em_frag, explosion->data.p_start);
	sound_play(explosion->param.snd);

	float mass = 1;
	explosion->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, explosion->param.size, cpvzero)));
	cpBodySetUserData(explosion->data.body, explosion);
	cpBodySetPos(explosion->data.body, explosion->data.p_start);

	cpShape *shape = we_add_circle_shape(current_space, explosion->data.body,explosion->param.size,0.8,0.2);
	we_shape_collision(shape, &this, LAYER_ENEMY | LAYER_PLAYER | LAYER_PICKUP | LAYER_BUILDING, &this);
	cpShapeSetSensor(shape, 1);

	cpSpaceAddConstraint(current_space, cpPivotJointNew(explosion->data.body, current_space->staticBody, explosion->data.p_start));

	explosion->shape = shape;
}

void explosion_create(cpVect p, EMITTER_ID em_expl, EMITTER_ID em_frag, Mix_Chunk *snd, float force, float size, float duration)
{
	obj_param_explosion param;
	param.em_expl = em_expl;
	param.em_frag = em_frag;
	param.snd = snd;
	param.force = force;
	param.seconds = duration;
	param.size = size;
	instance_create(obj_id_explosion, &param, p, cpvzero);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	float t = explosion->data.time_alive + 0.01;
	if (t > explosion->param.seconds) {
		instance_remove(&explosion->data);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	extern int debug_draw;
	if (debug_draw) {
		draw_color4f(0.5,0.5,0.5,0);
		draw_circle(RLAY_GUI_FRONT, explosion->data.body->p, explosion->param.size);
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &explosion->data.body);
}
