#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "chipmunk.h"

#define OBJ_NAME factory
#include "we_defobj.h"


static void remove_factory_from_tank(obj_tank *);

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(factory, HPBAR, &factory->hp_bar);
	COMPONENT_SET(factory, COINS, &factory->param.coins);
	COMPONENT_SET(factory, MINIMAP, &factory->radar_image);
	factory->radar_image = cmp_new_minimap(20, COL_BLUE);

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
	cpBodySetPos(factory->data.body, cpv(factory->data.p_start.x,64+size/2));

	shape_add_shapes(space, factory->param.shape_id, factory->data.body, 400, 1, 0.7, factory, &this, LAYER_BUILDING, 1);
	cpBodySetUserData(factory->data.body, factory);

	hpbar_init(&factory->hp_bar, factory->param.max_hp, 200, 35, 0, 180,
			&(factory->data.body->p));
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	factory->timer += dt;
	sprite_update(&(factory->data.spr));
	if (factory->timer > factory->param.spawn_delay && factory->cur < factory->param.max_tanks) {
		if(se_distance_to_player(factory->data.body->p.x) < factory->max_distance) {
			float x = factory->data.body->p.x;
			float y = factory->data.body->p.y - 150;
			instance * ins = instance_create(factory->param.type, factory->param.param,x,y,0,0);
			COMPONENT_SET(ins, CREATOR, factory);
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

static void remove_factory_from_child(instance *child, void *factory) {
	if (COMPONENT(child, CREATOR, void *) == factory) {
		COMPONENT_SET(child, CREATOR, NULL);
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	particles_get_emitter_at(EMITTER_FRAGMENTS, factory->data.body->p);
	se_spawn_coins((instance *)factory);
	instance_remove((instance *)factory);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	particles_release_emitter(factory->smoke);

	cpBodyEachShape(factory->data.body, se_shape_from_space, NULL);
	cpSpaceRemoveBody(space, factory->data.body);
	cpBodyFree(factory->data.body);

	instance_iterate_type((object_id *) factory->param.type, remove_factory_from_child, factory);
	instance_super_free((instance *)factory); //TODO move out to objects
}
