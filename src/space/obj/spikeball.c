/* standard c-libraries */
#include <stdio.h>
#include <math.h>

#include "object_types.h"
#define OBJ_NAME spikeball
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

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	sprite_create(&spikeball->data.spr, SPRITE_SPIKEBALL, 400, 400, 30);
	float radius = spikeball->param.radius;
	radius = 100;
	float mass = 10;
	spikeball->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(spikeball->data.body, cpv(spikeball->data.x, currentlvl->height-radius));

	spikeball->shape = se_add_circle_shape(spikeball->data.body,radius,0.8,0.2);

	cpShapeSetLayers(spikeball->shape, LAYER_TANK_FACTORY);
	cpShapeSetCollisionType(spikeball->shape, this.ID);
	cpBodySetUserData(spikeball->data.body, spikeball);

	spikeball->dolly = cpSpaceAddBody(space, cpBodyNew(10, INFINITY));
	cpSpaceAddShape(space, cpBoxShapeNew(spikeball->dolly, 30, 30));
	cpBodySetPos(spikeball->dolly, cpv(spikeball->data.x,currentlvl->height-100));

	spikeball->winch = cpSpaceAddConstraint(space, cpSlideJointNew(spikeball->dolly, spikeball->data.body, cpvzero, cpvzero, 0, INFINITY));
	cpConstraintSetMaxForce(spikeball->winch, 300000);
	cpConstraintSetMaxBias(spikeball->winch, 600);
	spikeball->winch_length = 100;
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	cpVect p = spikeball->data.body->p;

	cpBodySetVel(spikeball->dolly, cpv(0,0));

	float lvl_height = currentlvl->height;

	if(p.y > lvl_height - 300){
		spikeball->timer += dt;
		if(spikeball->timer > 1){
			spikeball->timer = 0;
			cpSlideJointSetMax(spikeball->winch, lvl_height-300);
		}
	}else if(p.y < 300){
		spikeball->timer += dt;
		if(spikeball->timer > 1){
			spikeball->timer = 0;
			cpSlideJointSetMax(spikeball->winch, 100);
		}
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	cpVect draw_pos = spikeball->data.body->p;
	cpVect a = spikeball->data.body->p;
	cpVect b = spikeball->dolly->p;
	draw_glow_line(a.x,a.y,b.x,b.y,50);
	sprite_render(&(spikeball->data.spr), &(draw_pos), cpvtoangle(spikeball->dolly->rot));

}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	cpSpaceRemoveShape(space, spikeball->shape);
	cpSpaceRemoveBody(space, spikeball->data.body);
	cpShapeFree(spikeball->shape);
	cpBodyFree(spikeball->data.body);
	cpBodyRemoveConstraint(spikeball->dolly,spikeball->winch);
	cpConstraintFree(spikeball->winch);
	instance_super_free((instance *)spikeball); //TODO move out to objects
}
