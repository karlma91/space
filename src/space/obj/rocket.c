/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Chipmunk physics library */
#include "chipmunk.h"

#include "object_types.h"
#define OBJ_NAME rocket
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

#include "../spaceengine.h"

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

	rocket->data.alive = 1;
	rocket->flame = particles_get_emitter(EMITTER_ROCKET_FLAME);
	rocket->angle = M_PI/2;

	cpFloat start_height;
	cpFloat height = 30;
	cpFloat mass = 2.0f;
	cpFloat width = 30;
	if (rocket->factory) {
		rocket->factory_id = rocket->factory->data.instance_id;
		start_height = rocket->factory->data.body->p.y;
	}else{
		start_height = 200 + 100;
	}

	// Make a car with some nice soft suspension
	cpVect boxOffset = cpv(0, 0);

	rocket->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPos(rocket->data.body , cpvadd(cpv(rocket->data.p_start.x, start_height), boxOffset));
	cpBodySetVelLimit(rocket->data.body,1000);
	rocket->shape = se_add_box_shape(rocket->data.body,width,height,0.7,0.0);

	//cpShapeSetGroup(tempShape, 1); // use a group to keep the car parts from colliding

	cpShapeSetLayers(rocket->shape, LAYER_ENEMY);
	cpShapeSetCollisionType(rocket->shape, &this);
	cpBodySetUserData(rocket->data.body, rocket);

	hpbar_init(&rocket->hp_bar, rocket->param.max_hp, 80, 16, -40, 60, &(rocket->data.body->p));
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

	cpBodySetForce(rocket->data.body,cpvzero);
	if (rocket->active) {
		target_angle = cpvtoangle(d);
		cpBodyApplyForce(rocket->data.body,cpvmult(cpvforangle(target_angle),rocket->param.force),cpvzero);
	} else {
		target_angle = M_PI / 2;
		cpBodyApplyForce(rocket->data.body,cpvmult(cpv(0,1),rocket->param.force),cpvzero);
	}

	rocket->angle = turn_toangle(rocket->angle, target_angle,1 * (2*M_PI* dt));
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,1,1,1);
	hpbar_draw(&rocket->hp_bar);
	draw_texture(rocket->param.tex_id, &(rocket->data.body->p),TEX_MAP_FULL,100, 100, rocket->angle*(180/M_PI));
}


static void shape_from_space(cpBody *body, cpShape *shape, void *data)
{
    cpSpaceRemoveShape(space, shape);
    cpShapeFree(shape);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	//TODO finn ut om rocket sprengte eller ikke
	//se_spawn_coins(object);
	cpBodyEachShape(rocket->data.body,shape_from_space,NULL);

	cpSpaceRemoveBody(space, rocket->data.body);
	cpBodyFree(rocket->data.body);

	particles_release_emitter(rocket->flame);
	if(rocket->factory != NULL){
		rocket->factory->cur--;
	}

	instance_super_free((instance *)rocket);
}
