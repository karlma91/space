/* header */
#include "tankfactory.h"

/* standard c-libraries */
#include <stdio.h>
#include <math.h>

/* Game state */
#include "space.h"

/* Drawing */
#include "draw.h"
#include "particles.h"

/* Game components */
#include "player.h"
#include "tank.h"

static void init(object *fac);
static void update(object *fac);
static void render(object *fac);
static void destroy(object *obj);
static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused);

struct obj_type type_tank_factory = {
	ID_TANK_FACTORY,
	init,
	update,
	render,
	destroy
};

static struct tank_factory *temp;

object *tankfactory_init( int x_pos , int max_tanks, float max_hp)
{
	struct tank_factory *fac = malloc(sizeof(struct tank_factory));
	fac->type = &type_tank_factory;
	fac->max = max_tanks;
	fac->cur = 0;
	fac->max_hp = max_hp;

	fac->max_hp = 200; //TMP
	fac->timer = 10;
	fac->hp = fac->max_hp;

	cpFloat size = 100;
	/* make and add new body */
	fac->body = cpSpaceAddBody(space, cpBodyNew(500, cpMomentForBox(500.0f, size, size)));
	cpBodySetPos(fac->body, cpv(x_pos,size));

	/* make and connect new shape to body */
	fac->shape = cpSpaceAddShape(space, cpBoxShapeNew(fac->body, size, size));
	cpShapeSetFriction(fac->shape, 1);

	//cpShapeSetGroup(fac->shape, 10);

	cpShapeSetLayers(fac->shape,2);

	cpShapeSetCollisionType(fac->shape, ID_TANK_FACTORY);
	cpSpaceAddCollisionHandler(space, ID_TANK_FACTORY, ID_PLAYER_BULLET, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(fac->body, (object*)fac);
	list_add((object*)fac);
	return (object*)fac;
}


static void init(object *fac)
{
	temp = ((struct tank_factory*)fac);
}

static void update(object *fac)
{
	temp = ((struct tank_factory*)fac);
	temp->timer+=dt;
	if(temp->timer > 10 && temp->cur < temp->max){
		temp->timer = 0;
		tank_init(temp, 100.0f);
		temp->cur += 1;
	}
}

static void render(object *fac)
{
	temp = ((struct tank_factory*)fac);

	glColor3f(1,1,1);
	draw_hp(temp->body->p.x-50, temp->body->p.y + 60, 100, 20, temp->hp / temp->max_hp);
	glColor3f(1,1,0);
	draw_boxshape(temp->shape,RGBAColor(0.2,0.9,0.1,1),RGBAColor(0.6,0.9,0.4,1));
}

static void postStepRemove(cpSpace *space, cpShape *shape, void *unused)
{
	cpSpaceRemoveBody(space, shape->body);
	//cpBodyFree(shape->body);

	cpSpaceRemoveShape(space, shape);
	//cpShapeFree(shape);
}

static int collision_player_bullet(cpArbiter *arb, cpSpace *space, void *unused)
{
	cpShape *a, *b;
	cpArbiterGetShapes(arb, &a, &b);

	temp = ((struct tank_factory*)(a->body->data));
	temp->hp -= 10;
	particles_add_explosion(b->body->p,0.3,1500,15,200);
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, b, NULL);
	if(temp->hp <=0 ){
		destroy(a->body->data);
		particles_add_explosion(a->body->p,1,2000,50,800);
		cpSpaceAddPostStepCallback(space, (cpPostStepFunc)postStepRemove, a, NULL);
	}

	return 0;
}


static void destroy(object *obj)
{
	*obj->remove = 1;
	free(obj);
}
