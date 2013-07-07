/* header */
#include "tankfactory.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

#include "game.h"
/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "particles.h"

#include "objects.h"
/* Game components */
#include "player.h"
#include "tank.h"
#include "rocket.h"
#include "bullet.h"
#include "spaceengine.h"
#include "texture.h"

static void init(object_group_factory *);
static void update(object_group_factory *);
static void render(object_group_factory *);
static void destroy(object_group_factory *);
static void remove_factory_from_tank(object_group_tank *);

object_group_preset type_tank_factory =
	{ ID_FACTORY,
			init,
			update,
			render,
			destroy
	};

object_group_factory *object_create_factory(int x_pos, object_param_factory *param) {
	object_group_factory *factory = (object_group_factory *)objects_super_malloc(ID_FACTORY, sizeof(*factory));
	factory->data.alive = 1;
	factory->data.preset = &type_tank_factory;
	factory->data.components.hp_bar = &(factory->hp_bar);
	factory->data.components.score = &(param->score);
	factory->data.components.body_count = 0;
	factory->param = param;

	factory->cur = 0;
	factory->rot = 0;

	static int randomness= 0;
	randomness += 123;
	randomness *= randomness;
	factory->timer = (factory->param->spawn_delay) * ((randomness % 0xFF + 160) / 400.0f + 0.2f);
	factory->max_distance = 900; //TODO read from object definition?
	//fac->hp = fac->param->max_hp; //TODO FIXME

	cpFloat size = 375;

	sprite_create(&factory->data.spr, factory->param->sprite_id, 400, 400, 30);

	if (factory->param->type == ID_TANK) {
		factory->smoke = particles_get_emitter(EMITTER_SMOKE);
		factory->data.spr.sub_index = rand() & 0x7;
	} else {

	}

	/* make and add new body */
	factory->data.body = cpSpaceAddBody(space,
			cpBodyNew(500, cpMomentForBox(5000.0f, size, size)));
	cpBodySetPos(factory->data.body, cpv(x_pos,64+size/2));

	/* make and connect new shape to body */
	factory->shape = cpSpaceAddShape(space,cpBoxShapeNew(factory->data.body, size, size));
	cpShapeSetFriction(factory->shape, 1);
	cpShapeSetElasticity(factory->shape, 0.7f);


	//cpShapeSetGroup(fac->shape, 10);

	cpShapeSetLayers(factory->shape, LAYER_TANK_FACTORY);

	cpShapeSetCollisionType(factory->shape, ID_FACTORY);

	cpBodySetUserData(factory->data.body, factory);
	objects_add((object_data *) factory);

	hpbar_init(&factory->hp_bar, param->max_hp, 200, 35, -50, 180,
			&(factory->data.body->p));

	return (object_group_factory*) factory;
}

static void init(object_group_factory *factory) {

}

static void update(object_group_factory *factory) {
	factory->timer += dt;
	sprite_update(&(factory->data.spr));
	if (factory->timer > factory->param->spawn_delay
			&& factory->cur < factory->param->max_tanks ) {

		if(factory->param->type == ID_ROCKET){
			if(se_distance_to_player(factory->data.body->p.x) < factory->max_distance){
				object_create_rocket(factory->data.body->p.x, factory,factory->param->r_param);
				factory->timer = 0;
				factory->cur += 1;
			}
		}else{
			factory->timer = 0;
			object_create_tank(factory->data.body->p.x, factory, factory->param->t_param);
			factory->cur += 1;
		}
	}
	if (factory->param->type == ID_TANK && factory->smoke) {
		factory->smoke->p.x = factory->data.body->p.x - 80;
		factory->smoke->p.y = factory->data.body->p.y + 255;
	}
}

static void render(object_group_factory *factory) {
	draw_color4f(1,1,1,1);

	//draw_boxshape(factory->shape,RGBAColor(0.2,0.9,0.1,1),RGBAColor(0.6,0.9,0.4,1));
	factory->rot += 381 * dt;
	cpVect draw_pos = factory->data.body->p;
	draw_pos.y += 8;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	float tmp_texmap[8] = {0,1, 1,1, 0,0, 1,0};
	//draw_texture(factory->param->tex_id, &(draw_pos), &tmp_texmap[0], 400, 400, 0);
	sprite_render(&(factory->data.spr), &(draw_pos), 0);

	hpbar_draw(&factory->hp_bar);
	draw_bar(factory->data.body->p.x+160,factory->data.body->p.y-150,40,150,factory->timer / factory->param->spawn_delay,0);
}

//FIXME Somewhat slow temporary fix, as objects_iterate_type does not support extra arguments!
static void remove_factory_from_tank(object_group_tank *tank) {
	if (tank->factory) {
		tank->factory = objects_by_id(ID_FACTORY,tank->factory_id);
	}
}
static void remove_factory_from_rocket(object_group_rocket *rocket) {
	if (rocket->factory) {
		rocket->factory = objects_by_id(ID_FACTORY,rocket->factory_id);
	}
}

static void destroy(object_group_factory *factory) {
	particles_get_emitter_at(EMITTER_FRAGMENTS, factory->data.body->p);
	particles_release_emitter(factory->smoke);

	cpSpaceRemoveShape(space, factory->shape);
	cpSpaceRemoveBody(space, factory->data.body);
	cpShapeFree(factory->shape);
	cpBodyFree(factory->data.body);
	objects_iterate_type(remove_factory_from_tank, ID_TANK);
	objects_iterate_type(remove_factory_from_rocket, ID_ROCKET);
	objects_super_free((object_data *)factory);
}
