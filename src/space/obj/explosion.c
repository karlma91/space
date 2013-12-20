
#include "object_types.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define OBJ_NAME explosion
#include "we_defobj.h"


static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	EMITTER_ID em_expl = particles_bind_emitter(explosion->param.em_expl);
	EMITTER_ID em_frag = particles_bind_emitter(explosion->param.em_frag);
	particles_get_emitter_at(RLAY_GAME_FRONT, em_expl, explosion->data.p_start);
	particles_get_emitter_at(RLAY_GAME_FRONT, em_frag, explosion->data.p_start);
	Mix_Chunk * snd = sound_loadchunk(explosion->param.snd);
	sound_play(snd);

	float mass = 1;
	explosion->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, 1, cpvzero)));
	cpBodySetUserData(explosion->data.body, explosion);
	cpBodySetPos(explosion->data.body, explosion->data.p_start);

	cpShape *shape = we_add_circle_shape(current_space, explosion->data.body,10,0.8,0.2);
	we_shape_collision(shape, &this, LAYER_ENEMY | LAYER_PLAYER | LAYER_PICKUP | LAYER_BUILDING, &this);
	explosion->shape = shape;
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	float mass = 1;
	float size = 400;
	float time = 0.3;
	float t = explosion->data.time_alive + 0.001;
	explosion->size = (t/time)*(t/time)*size;

	cpCircleShapeSetRadius(explosion->shape,explosion->size);
	cpBodySetMoment(explosion->data.body, cpMomentForCircle(mass, 0.0f, explosion->size, cpvzero));

	if (t > time) {
		instance_remove(&explosion->data);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);
	draw_circle(RLAY_GUI_FRONT, explosion->data.body->p, explosion->size);
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
