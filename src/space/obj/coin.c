#include "object_types.h"

#define OBJ_NAME coin
#include "../../engine/components/object.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define COIN_DAMPING 0.90f
#define COIN_RADIUS 20
#define COIN_SIZE 40
#define COIN_MASS 0.1f
#define COIN_FPS 15

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void vel_func(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	cpBodyUpdateVelocity(body, cpvzero, COIN_DAMPING, dt);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	sprite_create(&coin->data.spr, SPRITE_COIN, COIN_SIZE, COIN_SIZE, COIN_FPS);
	sprite_set_index(&coin->data.spr, rand() & 0xF);

	coin->data.body = cpSpaceAddBody(space, cpBodyNew(COIN_MASS, cpMomentForCircle(COIN_MASS, 0.0f, COIN_RADIUS, cpvzero)));
	cpBodySetUserData(coin->data.body, coin);
	cpBodySetPos(coin->data.body, coin->data.p_start);
	float angle = (1.0f*rand() / RAND_MAX) * M_PI*2;
	float force = 500 + (1.0f*rand() / RAND_MAX)*1000 ;
	coin->data.body->v = cpvmult(cpvforangle(angle), force);
	coin->shape = se_add_circle_shape(coin->data.body,COIN_RADIUS-5,0.8,0.2);
	coin->data.body->velocity_func = vel_func;
	cpShapeSetCollisionType(coin->shape, &this);
	cpShapeSetGroup(coin->shape, &this);
	cpShapeSetLayers(coin->shape, LAYER_PICKUP);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	sprite_update(&(coin->data.spr));

	if (coin->pulled) {
		instance *player = instance_first(obj_id_player);

		//TODO handle TMP magnet
		cpVect diff = se_distance_a2b((instance *)coin, player);
		float length = cpvlength(diff);
		if (length > 400) {
			diff = cpvmult(diff, 50/length);
		} else if (length > 100) {
			diff = cpvmult(diff, 0.5);
		} else {
			((obj_player *) player)->coins += 1;
			coin->data.alive = 0;
		}
		coin->data.body->v = cpvadd(coin->data.body->v, diff);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);
	cpVect pos = coin->data.body->p;
	sprite_render(&(coin->data.spr), &pos, 0);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{

	cpSpaceRemoveShape(space, coin->shape);
	cpShapeFree(coin->shape);

	cpSpaceRemoveBody(space, coin->data.body);
	cpBodyFree(coin->data.body);

	instance_super_free((instance*)coin);
}
