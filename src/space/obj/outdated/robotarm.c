/*
 * robotarm.c
 *
 *  Created on: Feb 9, 2013
 *      Author: Karl
 */

#if ROBOT_ARM_OBJECT

/* header */
#include "robotarm.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Chipmunk physics library */
#include "chipmunk.h"

/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "particles.h"

#include "objects.h"

/* static prototypes */
static void init(instance *fac);
static void update(instance *fac);
static void render(instance *fac);
static void destroy(instance *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);

obj_preset type_robotarm= {
	ID_ROBOTARM,
	init,
	update,
	render,
	destroy
};


static struct robotarm *temp;

instance *robotarm_init(float xpos,struct robotarm_param *pram)
{
	struct robotarm *ra = malloc(sizeof(struct robotarm));
	((instance *) ra)->TYPE = &type_robotarm;
	((instance *) ra)->alive = 1;
	ra->param = pram;
	ra->hp = ra->param->max_hp;
	ra->timer = 0;

	ra->segments = 3;
	ra->seg_length = 140;

	ra->x = malloc(sizeof(int)*ra->segments);
	ra->y = malloc(sizeof(int)*ra->segments);
	ra->angle = malloc(sizeof(int)*ra->segments);
	int i;
	for(i=0; i<ra->segments; i++){
		ra->x[i] = 0;
		ra->y[i] = 0;
		ra->angle[i] = 0;
	}


	cpFloat size = 50;
	/* make and add new body */
	((instance *) ra)->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, size, size)));
	cpBodySetPos(((instance *) ra)->body, cpv(xpos,size+10));
	cpBodySetVelLimit(((instance *) ra)->body,180);
	/* make and connect new shape to body */
	ra->shape = cpSpaceAddShape(space, cpBoxShapeNew(((instance *) ra)->body, size, size));
	cpShapeSetFriction(ra->shape, 0.01);
	cpShapeSetLayers(ra->shape,1<<20);
	cpShapeSetCollisionType(ra->shape, ID_ROBOTARM);
	cpSpaceAddCollisionHandler(space, ID_ROBOTARM, ID_BULLET_PLAYER, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(((instance *) ra)->body, (instance*)ra);
	objects_add((instance*)ra);
	return (instance*)ra;
}


static void init(instance *fac)
{
	temp = ((struct robotarm*)fac);
}

static void update(instance *obj)
{
	temp = ((struct robotarm*)obj);
	temp->timer +=dt;

	obj_player *player = ((obj_player*)instance_first(ID_PLAYER));

	float targetx = player->data.body->p.x;
	float targety = player->data.body->p.y;


	int i;
	for(i=0; i<temp->segments; i++){
		float dx = targetx - temp->x[i];
		float dy = targety - temp->y[i];
		temp->angle[i] = atan2f(dy,dx);
		targetx = targetx - cos(temp->angle[i])*temp->seg_length;
		targety = targety - sin(temp->angle[i])*temp->seg_length;
	}
	for(i=temp->segments-1; i>0; i--){
		temp->x[i-1] = temp->x[i] + cos(temp->angle[i])*temp->seg_length;
		temp->y[i-1] = temp->y[i] + sin(temp->angle[i])*temp->seg_length;
	}

}

static void render(instance *obj)
{
	temp = ((struct robotarm*)obj);
	draw_color4f(1,0,0,1);
	draw_quad_line(temp->x[0],temp->y[0],temp->x[0] + cos(temp->angle[0])*temp->seg_length, temp->y[0] + sin(temp->angle[0])*temp->seg_length, 20);
	int i;
	for(i=0; i<temp->segments-1; i++){
		draw_color4f(1*(1-(1.0f*i/(temp->segments-1))),0,0,1);
		draw_quad_line(temp->x[i],temp->y[i],temp->x[i+1],temp->y[i+1], 20);
	}
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	temp = ((struct robotarm*)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->data.alive = 0;

	//particles_get_emitter_at(EMITTER_EXPLOSION, b->body->p);

	if(temp->hp <=0 ){
		//a->body->data = NULL;
		//particles_get_emitter_at(EMITTER_EXPLOSION, b->body->p);
		//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
		//((object *) temp)->alive = 0;
	}else{
		temp->hp -= 10;
	}

	return 0;
}


static void destroy(instance *obj)
{
	temp = ((struct robotarm*)obj);
	free(temp->x);
	free(temp->y);
	free(temp->angle);
	cpSpaceRemoveBody(space, obj->body);
	cpSpaceRemoveShape(space, temp->shape);

	free(obj);
}
#endif
