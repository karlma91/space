#include "object_types.h"

#define OBJ_NAME coin
#include "../../engine/components/object.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"

#define COIN_DAMPING 0.98f
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
	cpBodySetPos(coin->data.body, coin->data.p_start);

	coin->shape = se_add_circle_shape(coin->data.body,COIN_RADIUS-5,0.8,0.2);
	coin->data.body->velocity_func = vel_func;
	cpShapeSetLayers(coin->shape, LAYER_PICKUP);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	sprite_update(&(coin->data.spr));
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
