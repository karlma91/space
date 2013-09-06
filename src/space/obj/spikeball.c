#include "object_types.h"
#include "../game.h"
#include "../states/space.h"
#include "../spaceengine.h"
#include "chipmunk.h"

#define OBJ_NAME spikeball
#include "we_defobj.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	sprite_create(&spikeball->data.spr, SPRITE_SPIKEBALL, 400, 400, 30);
	float radius = spikeball->param.radius;
	radius = 100;
	float mass = 100;

	spikeball->data.body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetUserData(spikeball->data.body, spikeball);
	cpBodySetPos(spikeball->data.body, cpv(spikeball->data.p_start.x, currentlvl->height-radius));

	cpSpace *shape = we_add_circle_shape(space, spikeball->data.body,radius,0.8,0.2);
	we_shape_collision(shape, &this, LAYER_BUILDING, CP_NO_GROUP);

	spikeball->dolly = cpSpaceAddBody(space, cpBodyNew(10, INFINITY));
	cpSpaceAddShape(space, cpBoxShapeNew(spikeball->dolly, 30, 30));
	cpBodySetPos(spikeball->dolly, cpv(spikeball->data.p_start.x,currentlvl->height));

	cpBody *static_body = cpSpaceGetStaticBody(space);
	spikeball->winch = cpSpaceAddConstraint(space, cpSlideJointNew(spikeball->data.body,static_body, cpvzero, cpv(spikeball->data.p_start.x,currentlvl->height), 0, INFINITY));
	cpConstraintSetMaxForce(spikeball->winch, 300000);
	cpConstraintSetMaxBias(spikeball->winch, 200);
	spikeball->winch_length = 100;
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	cpVect p = spikeball->data.body->p;

	cpBodySetVel(spikeball->dolly, cpv(0,0));

	float lvl_height = currentlvl->height;

	if(p.y > lvl_height - 200 && !spikeball->down ){
		spikeball->timer += dt;
		if(spikeball->timer > 2){
			spikeball->timer = 0;
			spikeball->down = 1;
			cpSlideJointSetMax(spikeball->winch, lvl_height);
		}
	}else if(p.y < 200){
		spikeball->timer += dt;
		if(spikeball->timer > 2){
			spikeball->timer = 0;
			spikeball->down = 0;
			cpSlideJointSetMax(spikeball->winch, 100);
		}
	}
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	cpVect draw_pos = spikeball->data.body->p;
	cpVect a = spikeball->data.body->p;
	cpVect b = spikeball->dolly->p;

	se_rect2arch(&a);
	se_rect2arch(&b);

	draw_glow_line(a.x,a.y,b.x,b.y,50);
	sprite_render(&(spikeball->data.spr), &(draw_pos), cpvtoangle(spikeball->dolly->rot));
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(space, &spikeball->data.body);
	instance_super_free((instance *)spikeball); //TODO move out to objects
}
