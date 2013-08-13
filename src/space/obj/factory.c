/* standard c-libraries */
#include <stdio.h>
#include <math.h>

#include "object_types.h"
#define OBJ_NAME factory
#include "../../engine/components/object.h"

#include "../game.h"
#include "../../engine/engine.h"
#include "../../engine/state/statesystem.h"
#include "../../engine/io/waffle_utils.h"

#include "../../engine/audio/sound.h"

/* Game state */
#include "../states/space.h"

/* Drawing */
#include "../../engine/graphics/draw.h"
#include "../../engine/graphics/particles.h"
#include "../../engine/graphics/texture.h"

#include "chipmunk.h"
#include "../spaceengine.h"

static void remove_factory_from_tank(obj_tank *);

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	factory->ins.alive = 1;
	factory->ins.components.hp_bar = &(factory->hp_bar);
	factory->ins.components.score = &(factory->param.score);

	static int randomness= 0;
	randomness += 123;
	randomness *= randomness;
	factory->timer = (factory->param.spawn_delay) * ((randomness % 0xFF + 160) / 400.0f + 0.2f);

	factory->max_distance = 900; //TODO read from object definition
	//fac->hp = fac->param.max_hp; //TODO FIXME

	cpFloat size = 375;

	sprite_create(&factory->ins.spr, factory->param.sprite_id, 400, 400, 30);

	if (factory->param.type == obj_id_tank) {
		factory->smoke = particles_get_emitter(EMITTER_SMOKE);
		factory->ins.spr.sub_index = rand() & 0x7;
	}

	/* make and add new body */
	factory->ins.body = cpSpaceAddBody(space,
			cpBodyNew(500, cpMomentForBox(5000.0f, size, size)));
	cpBodySetPos(factory->ins.body, cpv(factory->ins.x,64+size/2));

	/* make and connect new shape to body */
	factory->shape = cpSpaceAddShape(space,cpBoxShapeNew(factory->ins.body, size, size));
	cpShapeSetFriction(factory->shape, 1);
	cpShapeSetElasticity(factory->shape, 0.7f);

	//cpShapeSetGroup(fac->shape, 10);
	cpShapeSetLayers(factory->shape, LAYER_TANK_FACTORY);
	cpShapeSetCollisionType(factory->shape, obj_id_factory->ID);
	cpBodySetUserData(factory->ins.body, factory);

	hpbar_init(&factory->hp_bar, factory->param.max_hp, 200, 35, -50, 180,
			&(factory->ins.body->p));
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	factory->timer += dt;
	sprite_update(&(factory->ins.spr));
	if (factory->timer > factory->param.spawn_delay && factory->cur < factory->param.max_tanks) {
		if(se_distance_to_player(factory->ins.body->p.x) < factory->max_distance) {
			instance_create(factory->param.type, factory->param.param,factory->ins.body->p.x,0,0,0);
			factory->timer = 0;
			factory->cur += 1;
		}
	}

	if (factory->smoke) {
		factory->smoke->p.x = factory->ins.body->p.x - 80;
		factory->smoke->p.y = factory->ins.body->p.y + 255;
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);

	factory->rot += 381 * dt;
	cpVect draw_pos = factory->ins.body->p;
	draw_pos.y += 8;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	sprite_render(&(factory->ins.spr), &(draw_pos), 0);

	hpbar_draw(&factory->hp_bar);
	draw_bar(factory->ins.body->p.x+160,factory->ins.body->p.y-150,40,150,factory->timer / factory->param.spawn_delay,0);
}

//FIXME Somewhat slow temporary fix, as objects_iterate_type does not support extra arguments!
static void remove_factory_from_tank(obj_tank *tank) {
	if (tank->factory) {
		tank->factory = (obj_factory *)instance_by_id(obj_id_factory, tank->factory_id);
	}
}
static void remove_factory_from_rocket(obj_rocket *rocket) {
	if (rocket->factory) {
		rocket->factory = (obj_factory *)instance_by_id(obj_id_factory, rocket->factory_id);
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(EMITTER_FRAGMENTS, factory->ins.body->p);
	particles_release_emitter(factory->smoke);

	cpSpaceRemoveShape(space, factory->shape);
	cpSpaceRemoveBody(space, factory->ins.body);
	cpShapeFree(factory->shape);
	cpBodyFree(factory->ins.body);

	instance_iterate_type((void (*)(instance *))remove_factory_from_tank, obj_id_tank);
	instance_iterate_type((void (*)(instance *))remove_factory_from_rocket, obj_id_rocket);

	instance_super_free((instance *)factory); //TODO move out to objects
}
