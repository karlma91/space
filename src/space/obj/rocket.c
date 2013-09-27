#include "chipmunk.h"
#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"

#define OBJ_NAME rocket
#include "we_defobj.h"

#define ROCKET_SIZE 200
#define ROCKET_VEL_LIMIT 600

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	rocket->param.damage = 50; // TODO parse damage from file
	rocket->fuel = 7;

	COMPONENT_SET(rocket, HPBAR, &rocket->hp_bar);
	COMPONENT_SET(rocket, COINS, &rocket->param.coins);
	COMPONENT_SET(rocket, DAMAGE, &rocket->param.damage);
	COMPONENT_SET(rocket, MINIMAP, &rocket->radar_image);
	rocket->radar_image = cmp_new_minimap(5, COL_RED);

	rocket->flame = particles_get_emitter(current_particles, EMITTER_ROCKET_FLAME);
	particles_self_draw(rocket->flame, 1);

	cpFloat height = ROCKET_SIZE/8;
	cpFloat mass = 2.0f;
	cpFloat width = ROCKET_SIZE/2;

	rocket->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPos(rocket->data.body, rocket->data.p_start);
	cpBodySetVelLimit(rocket->data.body, ROCKET_VEL_LIMIT);
	cpBodySetUserData(rocket->data.body, rocket);
	se_velfunc(rocket->data.body, 1);
	se_tangent_body(rocket->data.body);
	cpBodySetAngle(rocket->data.body, rocket->data.body->a + WE_PI_2);

	cpShape *shape = we_add_box_shape(current_space, rocket->data.body,width,height,0.7,0.0);
	we_shape_collision(shape, &this, LAYER_ENEMY, CP_NO_GROUP);

	hpbar_init(&rocket->hp_bar, rocket->param.max_hp, 80, 18, 0, 60, &(rocket->data.body->p));
	sprite_create(&rocket->data.spr, sprite_link("rocket"), ROCKET_SIZE, ROCKET_SIZE, 1);
}


static void on_update(OBJ_TYPE *OBJ_NAME)
{
	rocket->timer +=dt;

	/* gets the player from the list */
	instance *player = (instance_first(obj_id_player));

	if (rocket->active) {
		float target_angle = cpvtoangle(se_dist_a2b((instance*)rocket, player));
		rocket->data.body->a = turn_toangle(rocket->data.body->a, target_angle, 0.4 * (WE_2PI * dt));
		cpBodySetAngle(rocket->data.body, rocket->data.body->a);
	} else if (SE_ALTITUDE(rocket->data.body->p) >= currentlvl->height * 0.35){
		rocket->active = 1;
	}

	if (rocket->timer > rocket->fuel) {
		rocket->param.force = 0;
	}

	cpBodySetForce(rocket->data.body, cpvmult(rocket->data.body->rot, rocket->param.force));

	if (rocket->flame) {
		rocket->flame->p = rocket->data.body->p;
		rocket->flame->angular_offset = rocket->data.body->a + WE_PI_2;
		rocket->flame->disable = (cpvlength(cpBodyGetForce(rocket->data.body)) < 0.1);
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	particles_draw_emitter(rocket->flame);

	draw_color4f(1, 0.2, 0.2, 1);
	sprite_render_body(&rocket->data.spr, rocket->data.body);

	hpbar_draw(&rocket->hp_bar, cpvtoangle(rocket->data.body->p));
}


static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(current_space, shape);
    cpShapeFree(shape);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	se_spawn_coins((instance *)rocket);
	instance_remove((instance *)rocket);
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	particles_release_emitter(rocket->flame);
	we_body_remove(current_space, &rocket->data.body);
	factory_remove_child(rocket);
}
