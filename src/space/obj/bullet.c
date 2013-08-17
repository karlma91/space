/* header */
#include "objects.h"

#include "object_types.h"
#define OBJ_NAME bullet
#include "../../engine/components/object.h"

#include "../game.h"
#include "../spaceengine.h"
#include "../states/space.h"
#include "../../engine/engine.h"


static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	bullet->data.alive = 1;
	COMPONENT_SET(bullet, DAMAGE, &(bullet->param.damage));

	sprite_create(&(bullet->data.spr), SPRITE_GLOW_DOT, 30, 30, 0);

	cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

	cpVect pos = {bullet->data.x, bullet->data.y};
	cpVect vel = {bullet->data.hs, bullet->data.vs};

	bullet->data.body = cpSpaceAddBody(space, cpBodyNew(1, moment));
	cpBodySetPos(bullet->data.body, pos); //FIXME
	cpBodySetUserData(bullet->data.body, (instance*) bullet);
	cpBodySetVel(bullet->data.body,vel); //3000 //FIXME
	bullet->data.body->velocity_func = bulletVelocityFunc;

	bullet->shape = se_add_circle_shape(bullet->data.body, 15, 1, 0);

	// Sets bullets collision type
	cpShapeSetCollisionType(bullet->shape, this.ID);
	cpShapeSetGroup(bullet->shape, 10);

	if (bullet->param.friendly) {
		cpShapeSetLayers(bullet->shape, LAYER_PLAYER_BULLET);
	} else {
		cpShapeSetLayers(bullet->shape, LAYER_ENEMY_BULLET);
	}

	bullet->energy = 750; // number of msec energy
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	if (bullet->energy < 0) {
		bullet->data.alive = 0;
	} else {
		bullet->energy -= mdt;
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float alpha = bullet->energy < 500 ? bullet->energy / 500 : 1;

	if (bullet->param.friendly) {
		draw_color4f(0.3, 0.3, 0.9, alpha);
	} else {
		draw_color4f(0.9, 0.3, 0.3, alpha);
	}

	cpShape *shape = bullet->shape;

	cpCircleShape *circle = (cpCircleShape *) shape;
	cpVect vel = cpBodyGetVel(cpShapeGetBody(shape));

	cpVect pos_from = circle->tc;
	cpVect pos_to = circle->tc;
	pos_to.x -= vel.x / 128;
	pos_to.y -= vel.y / 128;

#if EXPERIMENTAL_GRAPHICS
	se_rect2arch(&pos_from);
	se_rect2arch(&pos_to);
#endif
	draw_glow_line(pos_from.x, pos_from.y, pos_to.x, pos_to.y, 64); //40 = 4 * radius
}


/**
 * Velocity function to remove gravity
 */
static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpVect g = cpv(0, 0); //-1000 //200

	cpBodyUpdateVelocity(body, g, damping, dt);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	cpSpaceRemoveShape(space, bullet->shape);
	cpSpaceRemoveBody(space, bullet->data.body);
	cpShapeFree(bullet->shape);
	cpBodyFree(bullet->data.body);
	instance_super_free((instance *)bullet);
}
