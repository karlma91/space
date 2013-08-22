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
#include "../../engine/components/shape.h"

#include "chipmunk.h"
#include "../spaceengine.h"


static void remove_factory_from_tank(obj_tank *);

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	factory->data.components[CMP_HPBAR] = &(factory->hp_bar);
	factory->data.components[CMP_SCORE] = &(factory->param.score);
	factory->data.components[CMP_MINIMAP] = &(factory->radar_image);
	factory->radar_image.c.b = 1;
	factory->radar_image.size = 10;

	static int randomness= 0;
	randomness += 123;
	randomness *= randomness;
	factory->timer = (factory->param.spawn_delay) * ((randomness % 0xFF + 160) / 400.0f + 0.2f);

	factory->max_distance = 900; //TODO read from object definition
	//fac->hp = fac->param.max_hp; //TODO FIXME

	cpFloat size = 375;

	sprite_create(&factory->data.spr, factory->param.sprite_id, 400, 400, 30);

	if (factory->param.type == obj_id_tank) {
		factory->smoke = particles_get_emitter(EMITTER_SMOKE);
		factory->data.spr.sub_index = rand() & 0x7;
	}

	/* make and add new body */
	factory->data.body = cpSpaceAddBody(space,
			cpBodyNew(500, cpMomentForBox(5000.0f, size, size)));
	cpBodySetPos(factory->data.body, cpv(factory->data.x,64+size/2));

	polyshape_add_shapes(space, factory->param.shape_id, factory->data.body, 400, 1, 0.7, 11111, this.ID, LAYER_TANK_FACTORY);
	cpBodySetUserData(factory->data.body, factory);

	hpbar_init(&factory->hp_bar, factory->param.max_hp, 200, 35, -50, 180,
			&(factory->data.body->p));
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	factory->timer += dt;
	sprite_update(&(factory->data.spr));
	if (factory->timer > factory->param.spawn_delay && factory->cur < factory->param.max_tanks) {
		if(se_distance_to_player(factory->data.body->p.x) < factory->max_distance) {
			instance_create(factory->param.type, factory->param.param,factory->data.body->p.x,0,0,0);
			factory->timer = 0;
			factory->cur += 1;
		}
	}

	if (factory->smoke) {
		factory->smoke->p.x = factory->data.body->p.x - 80;
		factory->smoke->p.y = factory->data.body->p.y + 255;
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);

	factory->rot += 381 * dt;
	cpVect draw_pos = factory->data.body->p;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	sprite_render(&(factory->data.spr), &(draw_pos), 0);

	hpbar_draw(&factory->hp_bar);
	draw_bar(factory->data.body->p.x+160,factory->data.body->p.y-150,40,150,factory->timer / factory->param.spawn_delay,0);
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
	particles_get_emitter_at(EMITTER_FRAGMENTS, factory->data.body->p);
	particles_release_emitter(factory->smoke);

	cpBodyEachShape(factory->data.body, se_shape_from_space, NULL);
	cpSpaceRemoveBody(space, factory->data.body);
	cpBodyFree(factory->data.body);

	instance_iterate_type((void (*)(instance *))remove_factory_from_tank, obj_id_tank);
	instance_iterate_type((void (*)(instance *))remove_factory_from_rocket, obj_id_rocket);

	instance_super_free((instance *)factory); //TODO move out to objects
}
