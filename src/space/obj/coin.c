#include "object_types.h"

#define OBJ_NAME coin
#include "../../engine/components/object.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "../game.h"


static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	sprite_create(&coin->data.spr, SPRITE_COIN, 100, 100, 15);


	float mass = 1;
	float radius = 40;
	cpVect pos = cpv(coin->data.x, coin->data.y);

	coin->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(coin->data.body, pos);
	coin->shape = se_add_circle_shape(coin->data.body,radius,0.8,0.9);

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
