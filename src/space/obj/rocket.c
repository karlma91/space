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

/* Game components */
#include "objects.h"

#include "../spaceengine.h"

float damage = 50;

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	rocket->data.components[CMP_HPBAR] = &(rocket->hp_bar);
	rocket->data.components[CMP_SCORE] = &(rocket->param.score);
	rocket->data.components[CMP_DAMAGE] = &(damage);

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
	cpBodySetPos(rocket->data.body , cpvadd(cpv(rocket->data.x, start_height), boxOffset));
	cpBodySetVelLimit(rocket->data.body,1000);
	rocket->shape = se_add_box_shape(rocket->data.body,width,height,0.7,0.0);

	//cpShapeSetGroup(tempShape, 1); // use a group to keep the car parts from colliding

	cpShapeSetLayers(rocket->shape, LAYER_TANK);
	cpShapeSetCollisionType(rocket->shape, this.ID);
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
	obj_player *player = ((obj_player*)instance_first(obj_id_player));

	cpVect pl = player->data.body->p;
	cpVect rc = rocket->data.body->p;

	if(rc.y>currentlvl->height*0.35){
		rocket->active = 1;
	}

	cpFloat ptx = (pl.x-rc.x); //direct way
	cpFloat pltx = (rc.x - currentlvl->left + (currentlvl->right - pl.x));
	cpFloat prtx = (currentlvl->right - rc.x + (pl.x - currentlvl->left));
	if(fabs(ptx) < prtx && fabs(ptx) < pltx){
		ptx = ptx>0? 1:-1;
	}else if(pltx < prtx){
		pl.x -= currentlvl->right - currentlvl->left;
		ptx = -1;
	}else {
		pl.x += currentlvl->right - currentlvl->left;
		ptx = 1;
	}

	float target_angle;

	cpBodySetForce(rocket->data.body,cpvzero);
	if (rocket->active) {
		target_angle = get_angle(pl,rc);
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
	cpBodyEachShape(rocket->data.body,shape_from_space,NULL);

	cpSpaceRemoveBody(space, rocket->data.body);
	cpBodyFree(rocket->data.body);

	particles_release_emitter(rocket->flame);
	if(rocket->factory != NULL){
		rocket->factory->cur--;
	}

	instance_super_free((instance *)rocket);
}
