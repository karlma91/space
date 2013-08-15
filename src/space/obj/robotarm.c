#include "object_types.h"

#define OBJ_NAME robotarm
#include "../../engine/components/object.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Chipmunk physics library */
#include "chipmunk.h"

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

	robotarm->segments = 16;
	robotarm->seg_length = 70;

	robotarm->x = malloc(sizeof(int)*robotarm->segments);
	robotarm->y = malloc(sizeof(int)*robotarm->segments);
	robotarm->angle = malloc(sizeof(int)*robotarm->segments);
	int i;
	for(i=0; i<robotarm->segments; i++){
		robotarm->x[i] = 0;
		robotarm->y[i] = 0;
		robotarm->angle[i] = 0;
	}


	cpFloat size = 50;
	/* make and add new body */
	((instance *) robotarm)->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, size, size)));
	cpBodySetPos(((instance *) robotarm)->body, cpv(robotarm->data.x,size+10));
	cpBodySetVelLimit(((instance *) robotarm)->body,180);
	/* make and connect new shape to body */
	robotarm->shape = cpSpaceAddShape(space, cpBoxShapeNew(robotarm->data.body, size, size));
	cpShapeSetFriction(robotarm->shape, 0.01);
	cpShapeSetLayers(robotarm->shape,1<<20);
	cpShapeSetCollisionType(robotarm->shape, this.ID);

	cpBodySetUserData(((instance *) robotarm)->body, (instance*)robotarm);
}

static void on_update(OBJ_TYPE *OBJ_NAME)
{
	robotarm->timer += dt;

	obj_player *player = ((obj_player*)instance_first(obj_id_player));

	float targetx = player->data.body->p.x;
	float targety = player->data.body->p.y;


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
	draw_glow_line(v1.x,v1.y,v2.x,v2.y, 20);

	int i;
	for(i=0; i<robotarm->segments-1; i++){
		draw_color4f(1*(1-(1.0f*i/(robotarm->segments-1))),0,0,1);
		v1.x = robotarm->x[i];
		v1.y = robotarm->y[i];
		v2.x = robotarm->x[i+1];
		v2.y = robotarm->y[i+1];
		se_rect2arch(&v1);
		se_rect2arch(&v2);
		draw_glow_line(v1.x,v1.y,v2.x,v2.y, 50);
	}
}

static void on_destroy(OBJ_TYPE *OBJ_NAME)
{
	free(robotarm->x);
	free(robotarm->y);
	free(robotarm->angle);
	cpSpaceRemoveBody(space, robotarm->data.body);
	cpSpaceRemoveShape(space, robotarm->shape);

	instance_super_free((instance *)robotarm);
}
