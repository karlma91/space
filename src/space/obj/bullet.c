#include "object_types.h"
#include "../game.h"
#include "../spaceengine.h"
#include "../states/space.h"

#define OBJ_NAME bullet
#include "we_defobj.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(bullet, DAMAGE, &(bullet->param.damage));

	sprite_create(&(bullet->data.spr), SPRITE_GLOW_DOT, 30, 30, 0);

	cpFloat moment = cpMomentForCircle(1, 0, 5, cpvzero);

	bullet->data.body = cpSpaceAddBody(current_space, cpBodyNew(1, moment));
	cpBodySetPos(bullet->data.body, bullet->data.p_start); //FIXME
	cpBodySetUserData(bullet->data.body, (instance*) bullet);
	cpBodySetVel(bullet->data.body, bullet->data.v_start); //3000 //FIXME
	se_velfunc(bullet->data.body, 0);

	cpShape *shape = we_add_circle_shape(current_space, bullet->data.body, 15, 1, 0);
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
	if ((bullet->energy -= mdt) < 0) {
		instance_destroy(bullet);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float alpha = 1;
	if (bullet->data.destroyed) {
		alpha = bullet->energy / 250;
		if ((bullet->energy -= mdt) < 0) {
			instance_remove(bullet);
		}
	}

	if (bullet->param.friendly) {
		draw_color4f(0.3, 0.3, 0.9, alpha);
	} else {
		draw_color4f(0.9, 0.3, 0.3, alpha);
	}

	cpVect p1 = bullet->data.body->p;
	cpVect p2 = cpvadd(p1, cpvmult(bullet->data.body->v, 1.0/128));

	draw_glow_line(p1, p2, 64);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	bullet->param.damage = 0; // TODO remove collision for this body's shapes
	//TODO add fade tween/animation here?
	bullet->energy = 250; //TMP 250 ms fade
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &bullet->data.body);
}
