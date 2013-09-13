#include "chipmunk.h"
#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"

#define OBJ_NAME rocket
#include "we_defobj.h"

#define ROCKET_SIZE 150
#define ROCKET_VEL_LIMIT 600

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	rocket->param.damage = 50; // TODO parse damage from file

	COMPONENT_SET(rocket, HPBAR, &rocket->hp_bar);
	COMPONENT_SET(rocket, COINS, &rocket->param.coins);
	COMPONENT_SET(rocket, DAMAGE, &rocket->param.damage);
	COMPONENT_SET(rocket, MINIMAP, &rocket->radar_image);
	rocket->radar_image = cmp_new_minimap(5, COL_RED);

	rocket->flame = particles_get_emitter(parti, EMITTER_ROCKET_FLAME);
	rocket->angle = M_PI/2;

	cpFloat height = ROCKET_SIZE/8;
	cpFloat mass = 2.0f;
	cpFloat width = ROCKET_SIZE/2;

	rocket->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	rocket->data.body->velocity_func = space_velocity;
	cpBodySetUserData(rocket->data.body, rocket);
	cpBodySetPos(rocket->data.body, rocket->data.p_start);
	cpBodySetVelLimit(rocket->data.body, ROCKET_VEL_LIMIT);

	cpShape *shape = we_add_box_shape(space, rocket->data.body,width,height,0.7,0.0);
	we_shape_collision(shape, &this, LAYER_ENEMY, CP_NO_GROUP);

	hpbar_init(&rocket->hp_bar, rocket->param.max_hp, 80, 18, 0, 60, &(rocket->data.body->p));
}


static void on_update(OBJ_TYPE *OBJ_NAME)
{
	rocket->timer +=dt;

	//TODO FIXME: Will crash if no particles emitter available!!
	if (rocket->flame){
		rocket->flame->p = rocket->data.body->p;
		rocket->flame->angular_offset = rocket->angle * (180/M_PI)+90;
	}

	/* gets the player from the list */
	instance *player = (instance_first(obj_id_player));


	if(rocket->data.body->p.y > currentlvl->height * 0.35){
		rocket->active = 1;
	}

	cpVect d = se_distance_a2b((instance*)rocket, player);

	float target_angle;

	//cpBodySetForce(rocket->data.body,cpvzero);
	if (rocket->active) {
		target_angle = cpvtoangle(d);
		cpBodySetForce(rocket->data.body,cpvmult(cpvforangle(target_angle),rocket->param.force));
	} else {
		target_angle = M_PI / 2;
		cpBodySetForce(rocket->data.body,cpvmult(cpv(0,1),rocket->param.force));
	}

	rocket->angle = turn_toangle(rocket->angle, target_angle,1 * (2*M_PI* dt));
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);
	hpbar_draw(&rocket->hp_bar);
	draw_texture(rocket->param.tex_id, &(rocket->data.body->p),TEX_MAP_FULL,ROCKET_SIZE, ROCKET_SIZE, rocket->angle*(180/M_PI));
}


static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
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
	we_body_remove(space, &rocket->data.body);
	factory_remove_child(rocket);
	instance_super_free((instance *)rocket);
}
