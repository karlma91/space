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
	if (radius == 0) {
        radius = 100;
        spikeball->param.radius = radius;
    }
	float mass = 100;

	spikeball->data.body = cpSpaceAddBody(current_space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetUserData(spikeball->data.body, spikeball);
	cpBodySetPos(spikeball->data.body, cpv(spikeball->data.p_start.x, currentlvl->height-radius));

	cpShape *shape = we_add_circle_shape(current_space, spikeball->data.body,radius,0.8,0.2);
	we_shape_collision(shape, &this, LAYER_BUILDING, CP_NO_GROUP);

	spikeball->dolly = cpSpaceAddBody(current_space, cpBodyNew(10, INFINITY));
	cpBodySetPos(spikeball->dolly, cpv(spikeball->data.p_start.x,currentlvl->height));
	cpSpaceAddShape(current_space, cpBoxShapeNew(spikeball->dolly, 30, 30));
	cpBody *static_body = cpSpaceGetStaticBody(current_space);
	spikeball->winch = cpSpaceAddConstraint(current_space, cpSlideJointNew(spikeball->data.body,static_body, cpvzero, cpv(spikeball->data.p_start.x,currentlvl->height), 0, INFINITY));
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

static void on_update_dead(OBJ_TYPE *OBJ_NAME)
{
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	cpVect a = spikeball->data.body->p;
	cpVect b = spikeball->dolly->p;

	draw_glow_line(a, b, 50);
	sprite_render_body(RLAY_GAME_MID, &(spikeball->data.spr), spikeball->data.body);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
}

static void on_remove(OBJ_TYPE *OBJ_NAME)
{
	we_body_remove(current_space, &spikeball->data.body);
	we_body_remove(current_space, &spikeball->dolly);
}
