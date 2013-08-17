#include "object_types.h"

#define OBJ_NAME robotarm
#include "../../engine/components/object.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Chipmunk physics library */
#include "chipmunk.h"

#include "../game.h"

#include "../../engine/engine.h"
#include "../spaceengine.h"

/* Drawing */
#include "../../engine/graphics/draw.h"
#include "../states/space.h"

static void init(OBJ_TYPE *OBJ_NAME)
{
}

static void on_create(OBJ_TYPE *OBJ_NAME)
{
	robotarm->hp = robotarm->param.max_hp;
	robotarm->timer = 0;

	sprite_create(&(robotarm->saw_sprite), SPRITE_SAW, 300, 300, 30);
	sprite_create(&(robotarm->data.spr), SPRITE_TANK_BODY, 200, 100, 0);
	robotarm->segments = 4;
	robotarm->seg_length = currentlvl->height / (2*robotarm->segments);

	robotarm->x = 	  malloc(sizeof(int)*robotarm->segments);
	robotarm->y =	  malloc(sizeof(int)*robotarm->segments);
	robotarm->angle = malloc(sizeof(int)*robotarm->segments);
	int i;
	for(i=0; i<robotarm->segments; i++){
		robotarm->x[i] = robotarm->data.x;
		robotarm->y[i] = robotarm->data.y;
		robotarm->angle[i] = 0;
	}

	cpFloat radius = 100.0f;
	cpFloat mass = 5.0f;
	cpVect pos = cpv(robotarm->data.x, robotarm->data.y + 100);
	robotarm->saw = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPos(robotarm->saw, pos);
	cpBodySetVelLimit(robotarm->saw, 400);

	cpShape *shape = se_add_circle_shape(robotarm->saw, radius, 0.7, 0.0);
	cpShapeSetGroup(shape, 1);
	cpShapeSetLayers(shape, LAYER_TANK);
	cpShapeSetCollisionType(shape, this.ID);

	cpFloat size = 50;
	/* make and add new body */
	robotarm->data.body = cpBodyNew(200, cpMomentForBox(20.0f, size, size));
	cpBodySetPos(((instance *) robotarm)->body, cpv(robotarm->data.x,size+10));
	cpBodySetVelLimit(((instance *) robotarm)->body,180);

	/* make and connect new shape to body */
	robotarm->shape = cpSpaceAddShape(space, cpBoxShapeNew(robotarm->data.body, size, size));
	cpShapeSetFriction(robotarm->shape, 0.01);
	cpShapeSetLayers(robotarm->shape, LAYER_ENEMY_BULLET);
	cpShapeSetCollisionType(robotarm->shape, this.ID);

	cpBodySetUserData(((instance *) robotarm)->body, (instance*)robotarm);

	//connect sawblade with body
	cpSpaceAddConstraint(space, cpSlideJointNew(robotarm->saw, robotarm->data.body, cpv(0,0), cpv(0,0), 1.0f, (robotarm->segments)*robotarm->seg_length));
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	robotarm->timer += dt;
	sprite_update(&(robotarm->saw_sprite));


	float targetx = robotarm->saw->p.x;
	float targety = robotarm->saw->p.y;

	robotarm->x[robotarm->segments-1] = robotarm->data.body->p.x;
	robotarm->y[robotarm->segments-1] = robotarm->data.body->p.y;

	int i;
	for(i=0; i<robotarm->segments; i++){
		float dx = targetx - robotarm->x[i];
		float dy = targety - robotarm->y[i];
		robotarm->angle[i] = atan2f(dy,dx);
		targetx = targetx - cos(robotarm->angle[i])*robotarm->seg_length;
		targety = targety - sin(robotarm->angle[i])*robotarm->seg_length;
	}
	for(i=robotarm->segments-1; i>0; i--){
		robotarm->x[i-1] = robotarm->x[i] + cos(robotarm->angle[i])*robotarm->seg_length;
		robotarm->y[i-1] = robotarm->y[i] + sin(robotarm->angle[i])*robotarm->seg_length;
	}

	instance *player = instance_first(obj_id_player);
	cpVect d = se_distance_v(robotarm->saw->p, player->body->p);

	cpBodySetForce(robotarm->saw, cpvzero);
	d = cpvnormalize(d);
	cpBodyApplyForce(robotarm->saw, cpvmult(d, 10000), cpvzero);
}

static void on_render(OBJ_TYPE *OBJ_NAME)
{
	draw_color4f(1,0,0,1);
	cpVect v1 = cpv(0,0);
	cpVect v2 = cpv(0,0);

	v1.x = robotarm->x[0];
	v1.y = robotarm->y[0];
	v2.x = robotarm->x[0] + cos(robotarm->angle[0])*robotarm->seg_length;
	v2.y = robotarm->y[0] + sin(robotarm->angle[0])*robotarm->seg_length;
	se_rect2arch(&v1);
	se_rect2arch(&v2);
	draw_glow_line(v1.x,v1.y,v2.x,v2.y, 200);

	int i;
	for(i=0; i<robotarm->segments-1; i++){
		draw_color4f(1*(1-(1.0f*i/(robotarm->segments-1))),0,0,1);
		v1.x = robotarm->x[i];
		v1.y = robotarm->y[i];
		v2.x = robotarm->x[i+1];
		v2.y = robotarm->y[i+1];
		se_rect2arch(&v1);
		se_rect2arch(&v2);
		draw_glow_line(v1.x,v1.y,v2.x,v2.y, 200);
	}
	cpVect pos1 = robotarm->saw->p;
	cpVect pos2 = robotarm->data.body->p;
	sprite_render(&(robotarm->saw_sprite), &pos1, 0);
	sprite_render(&(robotarm->data.spr), &pos2, 0);
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	free(robotarm->x);
	free(robotarm->y);
	free(robotarm->angle);
	cpSpaceRemoveShape(space, robotarm->shape);
	cpShapeFree(robotarm->shape);

	//cpSpaceRemoveBody(space, robotarm->data.body);
	cpBodyFree(robotarm->data.body);

	instance_super_free((instance *)robotarm);
}
