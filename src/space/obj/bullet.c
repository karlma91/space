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

	float mass = bullet->param.mass;
	float radius = bullet->param.radius;

	fprintf(stderr, "creating bullet with mass: %f\n", mass);
	sprite_create(&bullet->data.spr, bullet->param.spr_id, radius*2, radius*2, 30);

	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	bullet->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, moment));
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

	bullet->energy = bullet->param.alive_time; // number of sec of energy
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	if ((bullet->energy -= dt) < 0) {
		instance_destroy((instance *)bullet);
	}
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
	if (bullet->data.time_destroyed > bullet->param.fade_time) {
		instance_remove((instance *)bullet);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	float alpha = 1;
	if (bullet->data.time_destroyed > 0) {
		alpha = maxf(0, 1 - bullet->data.time_destroyed / bullet->param.fade_time);
	}

	//TODO use color from param
	if (bullet->param.friendly) {
		draw_color4f(0, 1, 1, alpha);
	} else {
		draw_color4f(1, 0.4, 0.4, alpha);
	}
	if (bullet->param.render_stretch) {
		cpVect p1 = bullet->data.body->p;
		cpVect p2 = cpvadd(p1, cpvmult(bullet->data.body->v, 1.0/64));
		draw_glow_line(p1, p2, bullet->param.render_size);
	} else {
		sprite_render_body(RLAY_GAME_FRONT, &bullet->data.spr, bullet->data.body);
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	//TODO add sparks
	bullet->param.damage = 0; // TODO remove collision for this body's shapes
	bullet->energy = bullet->param.fade_time;
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &bullet->data.body);
}
