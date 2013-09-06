#include "object_types.h"
#include "../game.h"
#include "../spaceengine.h"
#include "../states/space.h"

#define OBJ_NAME bullet
#include "we_defobj.h"

static void bulletVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(bullet, DAMAGE, &(bullet->param.damage));

	sprite_create(&(bullet->data.spr), SPRITE_GLOW_DOT, 30, 30, 0);

	cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

	bullet->data.body = cpSpaceAddBody(space, cpBodyNew(1, moment));
	cpBodySetPos(bullet->data.body, bullet->data.p_start); //FIXME
	cpBodySetUserData(bullet->data.body, (instance*) bullet);
	cpBodySetVel(bullet->data.body, bullet->data.v_start); //3000 //FIXME
	bullet->data.body->velocity_func = bulletVelocityFunc;

	cpShape *shape = we_add_circle_shape(space, bullet->data.body, 15, 1, 0);
	cpShapeSetCollisionType(shape, &this);
	cpShapeSetGroup(shape, bullet);

	if (bullet->param.friendly) {
		cpShapeSetLayers(shape, LAYER_BULLET_PLAYER);
	} else {
		cpShapeSetLayers(shape, LAYER_BULLET_ENEMY);
	}

	bullet->energy = 750; // number of msec energy
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	if (bullet->energy < 0) {
		instance_remove(bullet);
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

	cpVect p1 = bullet->data.body->p;
	cpVect p2 = cpvadd(p1, cpvmult(bullet->data.body->v, 1.0/128));

#if EXPERIMENTAL_GRAPHICS
	se_rect2arch(&p1);
	se_rect2arch(&p2);
#endif

	draw_glow_line(p1.x, p1.y, p2.x, p2.y, 64);
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
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(space, &bullet->data.body);
	instance_super_free((instance *)bullet);
}