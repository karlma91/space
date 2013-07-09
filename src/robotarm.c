/*
 * robotarm.c
 *
 *  Created on: Feb 9, 2013
 *      Author: Karl
 */


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

/* Game components */
#include "player.h"
#include "bullet.h"

/* static prototypes */
static void init(object_data *fac);
static void update(object_data *fac);
static void render(object_data *fac);
static void destroy(object_data *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);

object_group_preset type_robotarm= {
	ID_ROBOTARM,
	init,
	update,
	render,
	destroy
};


static struct robotarm *temp;

object_data *robotarm_init(float xpos,struct robotarm_param *pram)
{
	struct robotarm *ra = malloc(sizeof(struct robotarm));
	((object_data *) ra)->preset = &type_robotarm;
	((object_data *) ra)->alive = 1;
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
	((object_data *) ra)->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, size, size)));
	cpBodySetPos(((object_data *) ra)->body, cpv(xpos,size+10));
	cpBodySetVelLimit(((object_data *) ra)->body,180);
	/* make and connect new shape to body */
	ra->shape = cpSpaceAddShape(space, cpBoxShapeNew(((object_data *) ra)->body, size, size));
	cpShapeSetFriction(ra->shape, 0.01);
	cpShapeSetLayers(ra->shape,1<<20);
	cpShapeSetCollisionType(ra->shape, ID_ROBOTARM);
	cpSpaceAddCollisionHandler(space, ID_ROBOTARM, ID_BULLET_PLAYER, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(((object_data *) ra)->body, (object_data*)ra);
	objects_add((object_data*)ra);
	return (object_data*)ra;
}


static void init(object_data *fac)
{
	temp = ((struct robotarm*)fac);
}

static void update(object_data *obj)
{
	temp = ((struct robotarm*)obj);
	temp->timer +=dt;

	object_group_player *player = ((object_group_player*)objects_first(ID_PLAYER));

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

static void render(object_data *obj)
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


static void destroy(object_data *obj)
{
	temp = ((struct robotarm*)obj);
	free(temp->x);
	free(temp->y);
	free(temp->angle);
	cpSpaceRemoveBody(space, obj->body);
	cpSpaceRemoveShape(space, temp->shape);

	free(obj);
}

