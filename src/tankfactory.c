/* header */
#include "tankfactory.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "particles.h"

#include "objects.h"
/* Game components */
#include "player.h"
#include "tank.h"
#include "bullet.h"
#include "spaceengine.h"

static void init(object_group_tankfactory *);
static void update(object_group_tankfactory *);
static void render(object_group_tankfactory *);
static void destroy(object_group_tankfactory *);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);
static void remove_factory_from_tank(object_group_tank *);

object_group_preset type_tank_factory =
	{ ID_TANK_FACTORY,
			init,
			update,
			render,
			destroy
	};

object_group_tankfactory *object_create_tankfactory(int x_pos, object_param_tankfactory *param) {
	object_group_tankfactory *factory = malloc(sizeof(*factory));
	factory->data.alive = 1;
	factory->data.preset = &type_tank_factory;
	factory->param = param;

	factory->cur = 0;
	factory->rot = 0;
	factory->smoke = particles_get_emitter(EMITTER_SMOKE);
	factory->timer = (factory->param->spawn_delay) * 0.7;
	//fac->hp = fac->param->max_hp; //TODO FIXME

	cpFloat size = 100;
	/* make and add new body */
	factory->data.body = cpSpaceAddBody(space,
			cpBodyNew(500, cpMomentForBox(5000.0f, size, size)));
	cpBodySetPos(factory->data.body, cpv(x_pos, size));

	/* make and connect new shape to body */
	factory->shape = cpSpaceAddShape(space,cpBoxShapeNew(factory->data.body, size, size));
	cpShapeSetFriction(factory->shape, 1);

	//cpShapeSetGroup(fac->shape, 10);

	cpShapeSetLayers(factory->shape, LAYER_TANK_FACTORY);

	cpShapeSetCollisionType(factory->shape, ID_TANK_FACTORY);
	cpSpaceAddCollisionHandler(space, ID_TANK_FACTORY, ID_BULLET_PLAYER,
			collision_player_bullet, NULL, NULL, NULL, NULL );

	cpBodySetUserData(factory->data.body, factory);
	objects_add((object_data *) factory);

	hpbar_init(&factory->hp_bar, param->max_hp, 100, 16, -50, 90,
			&(factory->data.body->p));

	return (object_group_tankfactory*) factory;
}

static void init(object_group_tankfactory *factory) {

}

static void update(object_group_tankfactory *factory) {
	factory->timer += dt;
	if (factory->timer > factory->param->spawn_delay
			&& factory->cur < factory->param->max_tanks) {
		factory->timer = 0;
		object_create_tank(factory->data.body->p.x, factory, factory->param->t_param);
		factory->cur += 1;
	}
	if (factory->smoke) {
		factory->smoke->p.x = factory->data.body->p.x - 20;
		factory->smoke->p.y = factory->data.body->p.y + 80;
	}
}

static void render(object_group_tankfactory *factory) {
	//glColor3f(1,1,1);

	hpbar_draw(&factory->hp_bar);

	if (factory->param->max_hp < 300)
		glColor3f(0.3, 0.6, 0.8);
	else
		glColor3f(0.8, 0.4, 0.4);

	//draw_boxshape(factory->shape,RGBAColor(0.2,0.9,0.1,1),RGBAColor(0.6,0.9,0.4,1));
	factory->rot += 381 * dt;
	float rot = factory->rot;

	draw_texture(TEX_WHEEL, &(factory->data.body->p), TEX_MAP_FULL, 150, 150, rot);
	draw_texture(factory->param->tex_id, &(factory->data.body->p), TEX_MAP_FULL, 200, 200, 0);
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused) {
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);

	object_group_tankfactory *factory = (object_group_tankfactory *) a->body->data;

	struct bullet *bt = ((struct bullet*) (b->body->data));

	bt->alive = 0;

	se_add_explotion_at_contact_point(arb);

	factory->hp_bar.value -= bt->damage;
	if (factory->hp_bar.value <= 0) {

		if (factory->data.alive) {
			particles_get_emitter_at(EMITTER_EXPLOSION, a->body->p);
			se_add_score_and_popup(b->body->p, factory->param->score);
		}
		factory->data.alive = 0;
	}
	return 0;
}

//FIXME Somewhat slow temporary fix, as objects_iterate_type does not support extra arguments!
static void remove_factory_from_tank(object_group_tank *tank) {
	if (tank->factory) {
		tank->factory = objects_by_id(ID_TANK_FACTORY,tank->factory_id);
	}
}

static void destroy(object_group_tankfactory *factory) {
	objects_remove(factory);
	particles_release_emitter(factory->smoke);

	cpSpaceRemoveShape(space, factory->shape);
	cpSpaceRemoveBody(space, factory->data.body);
	cpShapeFree(factory->shape);
	cpBodyFree(factory->data.body);
	objects_iterate_type(remove_factory_from_tank, ID_TANK);
	free(factory);
}
