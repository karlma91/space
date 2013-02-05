/* header */
#include "tank.h"

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

/* Game components */
#include "player.h"
#include "bullet.h"

/* static prototypes */
static void init(object *fac);
static void update(object *fac);
static void render(object *fac);
static void destroy(object *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);

struct obj_type type_tank= {
	ID_TANK,
	init,
	update,
	render,
	destroy
};


static struct tank *temp;

object *tank_init(struct tank_factory *fac, float max_hp)
{

	struct tank *tank = malloc(sizeof(struct tank));
	tank->type = &type_tank;
	tank->alive = 1;
	tank->max_hp = max_hp;
	tank->hp = tank->max_hp;
	tank->factory = fac;

	cpFloat size = 50;
	/* make and add new body */
	tank->body = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(20.0f, size, size)));
	cpBodySetPos(tank->body, cpv(fac->body->p.x,size+10));
	cpBodySetVelLimit(tank->body,180);
	/* make and connect new shape to body */
	tank->shape = cpSpaceAddShape(space, cpBoxShapeNew(tank->body, size, size));
	cpShapeSetFriction(tank->shape, 0.01);
	//cpShapeSetGroup(tank->shape, 10);
	cpShapeSetLayers(tank->shape,4);
	cpShapeSetCollisionType(tank->shape, ID_TANK);
	cpSpaceAddCollisionHandler(space, ID_TANK, ID_BULLET, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(tank->body, (object*)tank);
	list_add((object*)tank);
	return (object*)tank;
}


static void init(object *fac)
{
	temp = ((struct tank*)fac);
}

static void update(object *fac)
{
	temp = ((struct tank*)fac);

	cpFloat tx = temp->body->p.x;
	cpFloat px = player.body->p.x;

	cpFloat ptx = (px-tx); //direct way
	cpFloat pltx = (tx - level_left + (level_right - px));
	cpFloat prtx = (level_right - tx + (px - level_left));
	if(fabs(ptx) < prtx && fabs(ptx) < pltx){
		ptx = ptx>0? 1:-1;
	}else if(pltx < prtx){
		ptx = -1;
	}else {
		ptx = 1;
	}

	cpBodySetForce(temp->body,cpv(ptx*12000,0));

	if (temp->body->p.x < level_left + 50) temp->body->p.x = level_right - 50;
	if (temp->body->p.x > level_right - 50) temp->body->p.x = level_left + 50;
}

static void render(object *fac)
{
	temp = ((struct tank*)fac);

	glColor3f(1,1,1);
	draw_hp(temp->body->p.x-50, temp->body->p.y + 60, 100, 20, temp->hp / temp->max_hp);
	glColor3f(1,1,0);
	draw_boxshape(temp->shape,RGBAColor(0.8,0.3,0.1,1),RGBAColor(0.8,0.6,0.3,1));
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);
	temp = ((struct tank*)(a->body->data));

	struct bullet *bt = ((struct bullet*)(b->body->data));

	bt->alive = 0;

	particles_add_explosion(b->body->p,0.3,1500,15,200);

	if(temp->hp <=0 ){
		//a->body->data = NULL;
		particles_add_explosion(a->body->p,1,2000,50,800);
		//cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
		temp->alive = 0;
	}else{
		temp->hp -= 10;
	}

	return 0;
}


static void destroy(object *obj)
{
	temp = ((struct tank*)obj);
	cpSpaceRemoveBody(space, temp->body);
	cpSpaceRemoveShape(space, temp->shape);

	if(temp->factory != NULL){
		temp->factory->cur--;
	}

	*obj->remove = 1;
	free(obj);
}
