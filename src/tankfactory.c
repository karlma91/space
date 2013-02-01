#include "stdio.h"
#include "SDL.h"
#include "chipmunk.h"
#include "SDL_opengl.h"
#include "draw.h"
#include "font.h"
#include "main.h"
#include "particles.h"
#include "space.h"
#include "player.h"
#include "menu.h"
#include "math.h"
#include "tankfactory.h"
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

struct factory {
	struct obj_type *type;
	int id;
	cpBody *body;
	cpShape *shape;
	int max;
	cpFloat timer;
	int cur;
	cpFloat max_hp;
	cpFloat hp;
	int x_pos;
};

static struct factory *temp;

object *tankfactory_init( int x_pos , int max_tanks, float max_hp)
{
	struct factory *fac = malloc(sizeof(struct factory));
	fac->type = &type_tank_factory;
	fac->max = max_tanks;
	fac->max_hp = max_hp;

	fac->max_hp = 200; //TMP

	fac->hp = fac->max_hp;

	cpFloat size = 100;
	/* make and add new body */
	fac->body = cpSpaceAddBody(space, cpBodyNew(500, cpMomentForBox(500.0f, size, size)));
	cpBodySetPos(fac->body, cpv(x_pos,size));

	/* make and connect new shape to body */
	fac->shape = cpSpaceAddShape(space, cpBoxShapeNew(fac->body, size, size));
	cpShapeSetFriction(fac->shape, 1);

	cpShapeSetCollisionType(fac->shape, ID_TANK_FACTORY);
	cpSpaceAddCollisionHandler(space, ID_TANK_FACTORY, ID_PLAYER_BULLET, collision_player_bullet, NULL, NULL, NULL, NULL);

	cpBodySetUserData(fac->body, (object*)fac);
	return (object*)fac;
}


static void init(object *fac)
{
	temp = ((struct factory*)fac);
}

static void update(object *fac)
{
	temp = ((struct factory*)fac);
}

static void render(object *fac)
{
	temp = ((struct factory*)fac);

	glColor3f(1,1,1);
	draw_hp(temp->body->p.x-50, temp->body->p.y + 60, 100, 20, temp->hp / temp->max_hp);
	glColor3f(1,1,0);
	draw_boxshape(temp->shape);
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

	temp = ((struct factory*)(a->body->data));
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
	free(obj);
}
