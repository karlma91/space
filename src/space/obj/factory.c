#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "chipmunk.h"
#include "we_tween.h"
#define OBJ_NAME factory
#include "we_defobj.h"
#include "../../engine/components/shape.h"



static void init(OBJ_TYPE *OBJ_NAME)
{
	cpBodySetPos(factory->data.body, factory->data.p_start);
	se_tangent_body(factory->data.body);
	cpSpaceReindexShapesForBody(current_space, factory->data.body);
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	COMPONENT_SET(factory, HPBAR, &factory->hp_bar);
	COMPONENT_SET(factory, COINS, &factory->param.coins);
	COMPONENT_SET(factory, MINIMAP, &factory->radar_image);
	factory->radar_image = cmp_new_minimap(20, COL_BLUE);

	factory->timer = factory->param.spawn_delay * (we_randf * 0.8f + 0.2f);

	factory->max_distance = 900; //TODO read from object definition
	//fac->hp = fac->param.max_hp; //TODO FIXME

	factory->spawn_param = level_get_param(currentlvl->param_list, factory->param.spawn_type->NAME, factory->param.param_name);

	cpFloat size = 375;
	sprite_create(&factory->data.spr, factory->param.sprite, 400, 400, 30);

	if (factory->param.spawn_type == obj_id_tank) {
		factory->smoke = particles_get_emitter(RLAY_GAME_BACK, EM_SMOKE);
		particles_self_draw(factory->smoke, 1);
		factory->data.spr.sub_index = rand() & 0x7;
	}

	/* make and add new body */
	float m = 50; //500
	factory->data.body = cpSpaceAddBody(current_space,
			cpBodyNew(m, cpMomentForBox(m, size, size)));

	se_velfunc(factory->data.body, 1);

	shape_add_shapes(current_space, factory->param.shape, factory->data.body, 400, m, cpvzero, 1, 0.7, factory, &this, LAYER_BUILDING, 1);
	cpBodySetUserData(factory->data.body, factory);

	init(factory);

	hpbar_init(&factory->hp_bar, factory->param.max_hp, 200, 35, 0, 180,
			&(factory->data.body->p));
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	factory->timer += dt;
	sprite_update(&(factory->data.spr));
	if (factory->timer > factory->param.spawn_delay && factory->cur < factory->param.max_tanks) {
		instance *player = instance_nearest(factory->data.body->p, obj_id_player);
		if(cpvlength(cpvsub((factory->data.body->p),player->body->p)) < factory->max_distance) {
			cpVect pos = factory->data.body->p;
			pos = we_cart2pol(pos);
			pos.x += 100;
			pos = we_pol2cart(pos);

			instance * ins = instance_create(factory->param.spawn_type, factory->spawn_param, pos, cpvzero);
			COMPONENT_SET(ins, CREATOR, factory);
			factory->timer = 0;
			factory->cur += 1;
		}
	}

	if (factory->smoke) {
		cpVect pos = factory->data.body->p;
		cpVect rot = factory->data.body->rot;
		pos = cpvadd(pos,cpvrotate(cpv(-80,200),rot));
		factory->smoke->p = pos;
		factory->smoke->angular_offset = factory->data.body->a;
	}
}

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);
	factory->rot += 381 * dt;
	particles_draw_emitter(factory->smoke);
	sprite_render_body(RLAY_GAME_MID, &(factory->data.spr), factory->data.body);

	cpVect pos = cpvadd(factory->data.body->p, cpvrotate(cpv(160, -150), factory->data.body->rot));
	cpVect size = cpv(25, 100);
	draw_bar(RLAY_GAME_FRONT, pos, size, se_tangent(factory->data.body->p), factory->timer / factory->param.spawn_delay, 0);
	hpbar_draw(RLAY_GAME_FRONT, &factory->hp_bar, cpvtoangle(cpvperp(factory->data.body->p)));
}

static void remove_factory_from_child(instance *child, void *factory)
{
	if (COMPONENT(child, CREATOR, void *) == factory) {
		COMPONENT_SET(child, CREATOR, NULL);
	}
}

void factory_remove_child(instance *child)
{
	obj_factory *factory = COMPONENT(child, CREATOR, obj_factory*);
	if (factory && factory->data.TYPE == obj_id_factory) {
		--factory->cur;
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	explosion_create(factory->data.body->p, EM_EXPLOSIONBIG, EM_FRAGMENTS, SND_BUILDING_EXPLODE, 1300, 350, 0.3);
	se_spawn_coins((instance *)factory);
	instance_remove((instance *)factory);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	particles_release_emitter(factory->smoke);
	we_body_remove(current_space, &factory->data.body);
	instance_iterate_type((object_id *) factory->param.spawn_type, remove_factory_from_child, factory);
}
